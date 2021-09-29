// tftp server
// 4 byte header, 512 byte data,  network byte order

#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

#define swap2 __builtin_bswap16

#define TFTP_PORT 69
#define LOCAL_PORT 8888

#define TFTP_MAX_PAYLOAD_SIZE 512
#define TFTP_HEADER_LENGTH    4

#define TFTP_RRQ   1
#define TFTP_WRQ   2
#define TFTP_DATA  3
#define TFTP_ACK   4
#define TFTP_ERROR 5

enum tftp_error {
  TFTP_ERROR_FILE_NOT_FOUND    = 1,
  TFTP_ERROR_ACCESS_VIOLATION  = 2,
  TFTP_ERROR_DISK_FULL         = 3,
  TFTP_ERROR_ILLEGAL_OPERATION = 4,
  TFTP_ERROR_UNKNOWN_TRFR_ID   = 5,
  TFTP_ERROR_FILE_EXISTS       = 6,
  TFTP_ERROR_NO_SUCH_USER      = 7
};

#define TFTP_TIMEOUT_MSECS    5000
#define TFTP_MAX_RETRIES      5

#define IDLE 0
#define WRITING 1
#define READING 2

static uint32_t state, last_ms, last_lth, blknum, retries, mode_write, init;
static uint32_t nbytes, txpkts, rxpkts, retxpkts;
IPAddress tftp_server;

#define MODE "octet"


// tftp packet  4-byte header 512-byte payload
static uint16_t pktin[256], pktout[258];
static uint8_t *pin = (uint8_t *) pktin;
static uint8_t *pout = (uint8_t *) pktout;
static uint8_t *pbufin = (uint8_t *) &pktin[2];
static uint8_t *pbufout = (uint8_t *) &pktout[2];
static utin8_t *inbuff;
static int send_lth;

EthernetUDP Udp;

void close_connection() {
 state = IDLE;
}

void send_error(IPAddress host, int port, int code, char *msg) {
  int n = strlen(msg) + 1 + TFTP_HEADER_LENGTH;
  pktout[0] = swap2(TFTP_ERROR);
  pktout[1] = swap2(code);
  strcpy(pbufout, msg);
  Udp.beginPacket(host, port);
  Udp.write(pout, n);
  Udp.endPacket();
  Serial.printf("error %d %s\n", code, msg);
}


void resend() {
  Udp.beginPacket(tftp_server, TFTP_PORT);
  Udp.write(pout, last_lth);
  Udp.endPacket();
  txpkts++;
}


void send_ack(int blknum) {
  pktout[0] = swap2(TFTP_ACK);
  pktout[1] = swap2(blknum);
  last_lth = TFTP_HEADER_LENGTH;
  Udp.beginPacket(tftp_server, TFTP_PORT);
  Udp.write(pout, last_lth);
  Udp.endPacket();
  last_ms = millis();
  txpkts++;
}

void send_data() {
  int ret;

  pktout[0] = swap2(TFTP_DATA);
  pktout[1] = swap2(blknum);
  ret = tftp_state.ctx->read(tftp_state.handle, pbufout, TFTP_MAX_PAYLOAD_SIZE);
  if (ret < 0) {
    send_error(&tftp_state.addr, tftp_state.port, TFTP_ERROR_ACCESS_VIOLATION, "Error occured while reading the file.");
    close_connection();
    return;
  }
  send_lth = ret + TFTP_HEADER_LENGTH;
  // Serial.printf("send blk %d send_lth %d\n", tftp_state.blknum, send_lth);
  last_ms = millis();

  resend();
}

int recv() {
  int rlth, port;
  uint16_t op, blk;
  IPAddress remote;

  rlth = Udp.read(pin, sizeof(pktin));
  remote = Udp.remoteIP();
  port = Udp.remotePort();
  op = swap2(pktin[0]);
  blk = swap2(pktin[1]);
  rxpkts++;
  //  Serial.printf("rlth %d op %d blk %d\n", rlth, op, blk);
  if (state == IDLE) {
    send_error(remote, port, TFTP_ERROR_ILLEGAL_OPERATION, "unexpected pkt");
    return -1
  }
  if (remote != tftp_server) {
    send_error(remote, port, TFTP_ERROR_UNKNOWN_TRFR_ID, "Wrong server IP");
    return -1
  }


  switch (op) {
    case TFTP_ACK:
      {
        if (state == IDLE) {
          send_error(remote, port, TFTP_ERROR_ACCESS_VIOLATION, "No connection");
          return -1;
        }

        if (state != WRITING) {
          send_error(remote, port, TFTP_ERROR_ACCESS_VIOLATION, "Not a write connection");
          return -1;
        }

        if (blk != blknum) {
          send_error(remote, port, TFTP_ERROR_UNKNOWN_TRFR_ID, "Wrong block number");
          return -1;
        }

        int lastpkt = 0;

        if (send_lth != 0) {
          lastpkt = send_lth != (TFTP_MAX_PAYLOAD_SIZE + TFTP_HEADER_LENGTH);
        }
        if (!lastpkt) {
          blknum++;
          send_data();
        } else {
          close_connection();
        }
        return send_lth - TFTP_HEADER_LENGTH;
      }

    case TFTP_DATA:
      {
        if (state == IDLE) {
          send_error(remote, port, TFTP_ERROR_ACCESS_VIOLATION, "No connection");
          break;
        }

        if (sate != READING) {
          send_error(remote, port, TFTP_ERROR_ACCESS_VIOLATION, "Not a read connection");
          break;
        }


          memcpy(inbuff, pbuf_in,rlth - TFTP_HEADER_LENGTH)l
          send_ack(blk);
  

        if ((rlth - TFTP_HEADER_LENGTH) < TFTP_MAX_PAYLOAD_SIZE) {
          close_connection();
        }
        return (rlth - TFTP_HEADER_LENGTH);
      }

    case TFTP_RRQ:
    case TFTP_WRQ:
    default:
      send_error(remote, port, TFTP_ERROR_ILLEGAL_OPERATION, "Illegal operation");
      break;
  }
}


int tftp_open(IPAdress server, char *filename, int write_flag) {
   if (!init) {
    init = 1;
    Udp.begin(LOCAL_PORT);
  }
  mode_write = write_flag;
  nbytes =  txpkts = rxpkts = retxpkts = last_lth = 0;
  blknum = 0;
  state = IDLE;
  tftp_server = server;
}

int tftp_write(uint8_t *buff, int bytes) {
  if (state != WRITING || bytes < 0) return -1;
  if (bytes > TFTP_MAX_PAYLOAD_SIZE) bytes = TFTP_MAX_PAYLOAD_SIZE;
  memcpy(pbufout, buff, bytes);
  last_lth = bytes + TFTP_HEADER_LENGTH;
  nbytes += bytes;
  send_data();
}

int tftp_read(uint8_t *buff) {
  inbuff = buff;
  if (state == IDLE) return 0;
}

void tftp_close() {
  if (state == WRITING && lastlth == TFTP_MAX_PAYLOAD_SIZE ) {
    // write 0-lth buffer to show done
    tftp_put(pout, 0);
  }
  state = IDLE;
  Serial.printf("closed.%d bytes tx %d  rx %d  retx %d\n", 
    nbytes,txpkts, rxpkts, retxpkts);
}
