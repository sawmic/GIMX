/*
 Copyright (c) 2012 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef GPP_CON_H_
#define GPP_CON_H_

int gpp_connect();

int gpp_send(int axis[AXIS_MAX]);

void gpp_disconnect();

#endif /* GPP_CON_H_ */
