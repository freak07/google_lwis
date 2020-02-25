/*
 * Google LWIS Interrupt Handler
 *
 * Copyright (c) 2018 Google, LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef LWIS_INTERRUPT_H_
#define LWIS_INTERRUPT_H_

#include <linux/hashtable.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/platform_device.h>

#define EVENT_INFO_HASH_BITS 8

struct lwis_interrupt {
	int irq;
	char *name;
	/* Device that owns this interrupt */
	struct lwis_device *lwis_dev;
	/* Spinlock to lock acccess to this struct */
	spinlock_t lock;
	/* Flag if the event info has been set */
	bool has_events;
	/* BID of the register space where the status/reset/mask for this ISR
	 * can be accessed */
	int irq_reg_bid;
	/* Offset of the source register */
	int64_t irq_src_reg;
	/* Offset of the clear/reset register */
	int64_t irq_reset_reg;
	/* Offset of the mask register */
	int64_t irq_mask_reg;
	/* IRQ register bitwidth, in case there is a need to override the
	 * default */
	int irq_reg_bitwidth;
	/* If mask_reg actually disable the interrupts. */
	bool mask_toggled;
	/* Hash table of event info*/
	DECLARE_HASHTABLE(event_infos, EVENT_INFO_HASH_BITS);
	/* List of enabled events */
	struct list_head enabled_event_infos;
};

/*
 *  struct lwis_interrupt_list
 *  This is to store the list of interrupts the LWIS device uses
 */
struct lwis_interrupt_list {
	struct lwis_interrupt *irq;
	int count;
	/* Device that owns this interrupt list */
	struct lwis_device *lwis_dev;
};

/*
 *  lwis_interrupt_list_alloc: Allocate an instance of the lwis_interrupt_list
 *  and initialize the data structures according to the number of interrupts
 *  specified.
 */
struct lwis_interrupt_list *
lwis_interrupt_list_alloc(struct lwis_device *lwis_dev, int count);

/*
 *  lwis_interrupt_list_free: Deallocate the lwis_interrupt_list structure.
 */
void lwis_interrupt_list_free(struct lwis_interrupt_list *list);

/*
 *  lwis_interrupt_get: Register the interrupt by name.
 *  Returns: index number (>= 0) if success, -ve if error
 */
int lwis_interrupt_get(struct lwis_interrupt_list *list, int index, char *name,
		       struct platform_device *plat_dev);

/*
 * lwis_interrupt_set_event_info: Provides event-info structure for a given
 * interrupt based on index
 *
 * Takes ownership of (and later frees) irq_events and int_reg_bits
 * Does not free irq_reg_space
 * Returns: 0 on success
 */
int lwis_interrupt_set_event_info(struct lwis_interrupt_list *list, int index,
				  const char *irq_reg_space, int irq_reg_bid,
				  int64_t *irq_events, size_t irq_events_num,
				  uint32_t *int_reg_bits,
				  size_t int_reg_bits_num, int64_t irq_src_reg,
				  int64_t irq_reset_reg, int64_t irq_mask_reg,
				  bool mask_toggled, int irq_reg_bitwidth);

/*
 * lwis_interrupt_event_enable: Handles masking and unmasking interrupts when
 * an event is enabled or disabled
 *
 * Locks: May lock list->irq[index].lock
 * Alloc: No
 * Returns: 0 on success (event enabled/disabled)
 *          -EINVAL if event not known to this list
 */
int lwis_interrupt_event_enable(struct lwis_interrupt_list *list,
				int64_t event_id, bool enabled);

/*
 *  lwis_interrupt_request_all_default: Request all interupts in the list with
 *  the default IRQ handler provided in the C file.
 *  Returns: 0 if success, -ve if error
 */
int lwis_interrupt_request_all_default(struct lwis_interrupt_list *list);

/*
 *  lwis_interrupt_request_by_idx: Request interrupt by index, which also
 *  register an interrupt handler.
 *  Returns: 0 if success, -ve if error
 */
int lwis_interrupt_request_by_idx(struct lwis_interrupt_list *list, int index,
				  irq_handler_t handler, void *dev);

/*
 *  lwis_interrupt_request_by_name: Request interrupt by name, which also
 *  register an interrupt handler.
 *  Returns: 0 if success, -ve if error
 */
int lwis_interrupt_request_by_name(struct lwis_interrupt_list *list, char *name,
				   irq_handler_t handler, void *dev);

/*
 *  lwis_interrupt_free_all_default: Free all interrupts in the list.
 */
void lwis_interrupt_free_all_default(struct lwis_interrupt_list *list);

/*
 *  lwis_interrupt_free_by_idx: Free interrupt by index.
 */
void lwis_interrupt_free_by_idx(struct lwis_interrupt_list *list, int index,
				void *dev);

/*
 *  lwis_interrupt_free_by_name: Free interrupt by name.
 */
void lwis_interrupt_free_by_name(struct lwis_interrupt_list *list, char *name,
				 void *dev);

/*
 *  lwis_interrupt_print: Debug function to print all the interrupts in the
 *  supplied list.
 */
void lwis_interrupt_print(struct lwis_interrupt_list *list);

#endif /* LWIS_INTERRUPT_H_ */
