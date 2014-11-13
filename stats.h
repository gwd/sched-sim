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
#ifndef _STATS_H
#define _STATS_H

typedef unsigned long long tsc_t;

struct cycle_summary {
    int count;
    unsigned long long cycles;
    long long *sample;
};

void set_sample_size(int n);
void update_cycles(struct cycle_summary *s, long long c);
void print_cycle_summary(struct cycle_summary *s,
                         tsc_t total, char *p);
#endif
