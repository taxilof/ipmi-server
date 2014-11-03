ipmi-server
===========

open source ipmi server implementation for power monitoring

## files
 * **asf.c:** ASF Ping and Pong
 * **config.h:** configuration file
 * **ipmi-msg.c:** For handling all IPMI Commands with IPMI Messages
 * **ipmi-sdr.c:** Sensor Data Repository related code
 * **ipmi-sensor.c:** Sensor intitalisations
 * **ipmi-session.c:** IPMI Session Handshake and auth
 * **rmcp.c:** RMCP implementation
 * **udp-server.c:** main file for UDP Socket generation

## installation
 * `make`
 * `./ipmi-server` as root (because port 623 is used)
