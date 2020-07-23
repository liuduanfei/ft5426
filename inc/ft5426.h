/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-07-22     liuduanfei   the first version
 */

#ifndef _FT5426_H_
#define _FT5426_H_

#include "touch.h"

int rt_hw_ft5426_init(const char *name, struct rt_touch_config *cfg);

#endif /* _FT5426_H_ */
