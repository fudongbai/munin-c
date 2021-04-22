/*
 * Copyright (C) 2008-2013 Helmut Grohne <helmut@subdivi.de> - All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2 or v.3.
 */

/* This plugin is compatible with munin-mainline version 2.0.37. */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "common.h"
#include "plugins.h"

#define PROC_INTERRUPTS "/proc/interrupts"

struct irq {
	uint16_t irq;
	uint32_t count;
	char name[256];
};

int parse_interrupts(struct irq *_irq)
{
	FILE *f;
	char buff[256];
	int i = 0;

	if (!(f = fopen(PROC_INTERRUPTS, "r")))
		return fail("cannot open " PROC_INTERRUPTS);
	while (fgets(buff, 256, f)) {
		char name[256];
		uint32_t irq, count;
		if (!sscanf(buff, "%d: %d - %s", &irq, &count, name))
			continue;

		_irq[i].irq = irq;
		_irq[i].count = count;
		strncpy(_irq[i].name, name, sizeof(name));
		i++;
	}
	fclose(f);
	return i;
}

int irqstats(int argc, char **argv)
{
	struct irq irq[256];
	int num_of_irq;

	memset(irq, 0, sizeof(struct irq));
	num_of_irq = parse_interrupts(irq);

	if (argc > 1) {
		if (!strcmp(argv[1], "config")) {
			puts("graph_title Individual interrupts\n" "graph_args --base 1000 --logarithmic\n" "graph_vlabel interrupts / ${graph_period}\n" "graph_category system\n" "graph_info Shows the number of different IRQs received by the kernel. High disk or network traffic can cause a high number of interrupts (with good hardware and drivers this will be less so). Sudden high interrupt activity with no associated higher system activity is not normal.");
			puts("graph_order system user nice idle iowait irq softirq");

			for (int i = 0; i < num_of_irq; i++) {
				printf("%s.label %s\n", irq[i].name, irq[i].name);
				printf("%s.type DERIVE\n", irq[i].name);
				printf("%s.info Interrupt %d, for device(s): %s\n", irq[i].name, irq[i].irq, irq[i].name);
			}

			return 0;
		}
		if (!strcmp(argv[1], "autoconf"))
			return autoconf_check_readable(PROC_STAT);
	}

	for (int i = 0; i < num_of_irq; i++)
		printf("%s.value %d\n", irq[i].name, irq[i].count);

	return 0;
}
