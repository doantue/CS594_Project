#ifndef COMMON_INCLUDE_FILE
#define COMMON_INCLUDE_FILE

#define DEFAULT_BUFLEN   512
#define DEFAULT_PORT     "27015"
#define COMMAND_LENGTH   3
#define ERR_CODE_LENGTH  4
#define NUM_LIST_LENGTH  3
#define PAYLOAD_LENGTH   4
#define ERROR_RES        "001"
#define LIST_ROOMS_RES   "002"
#define LIST_MEMBERS_RES "003"
#define SEND_MSG_RES     "004"
#define DISCONNECT_RES   "005"
#define CONNECT_RES	     "006"
#define SUCCESS_RES	     "007"
#define CONNECT_REQ      "101"
#define CREATE_ROOM_REQ  "102"
#define LIST_ROOM_REQ    "103"
#define JOIN_ROOM_REQ    "104"
#define LEAVE_ROOM_REQ   "105"
#define LIST_MEMBER_REQ  "106"
#define SEND_MSG_REQ     "107"
#define DISCONNECT_REQ   "108"
#define HEARTBEAT_REQ    "109"

#define ERR_ROOM_EXIST   "0011"
#define ERR_ROOM_N_EXIST "0012"
#define ERR_ROOM_INV     "0013"

#define ERR_DATA_CORRUPT "0091"

#endif