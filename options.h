/*
    Copyright (C) 2010 Citrix Systems UK Ltd
    Author: George Dunlap

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef _OPTIONS_H
#define _OPTIONS_H
struct options {
    int time_limit;
    int pcpu_count;
    const struct workload * workload;
    const struct scheduler * scheduler;
} opt;

extern struct options opt;
void parse_options(int argc, char * argv[]);

#endif
