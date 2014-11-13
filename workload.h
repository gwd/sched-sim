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
#ifndef __WORKLOAD_H
#define __WORKLOAD_H
struct sim_phase {
    enum { PHASE_RUN, PHASE_BLOCK } type;
    int time;
};

#define MAX_VMS 16
#define MAX_PHASES 16
struct vm_workload {
    int phase_count;
    const struct sim_phase list[MAX_PHASES];
};

struct workload {
    const char * name;
    int vm_count;
    const struct vm_workload vm_workloads[MAX_VMS];
};

extern const int default_workload;
extern struct workload builtin_workloads[];
#endif
