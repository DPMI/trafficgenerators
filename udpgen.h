/* Common definitions for udpClient/Server */
/*   */
/* Tahir.Nawaz.Minhaz@bth.se */
/* Patrik.Arlos@bth.se */
/* */


typedef struct{
  u_int32_t exp_id;
  u_int32_t run_id;
  u_int32_t key_id;
  u_int32_t counter;
  u_int64_t starttime;
  u_int64_t stoptime;
  timeval depttime;
  u_int64_t recvstarttime;
  u_int64_t recvstoptime;
  timeval recvtime;
  char junk[1484];
}transfer_data;
