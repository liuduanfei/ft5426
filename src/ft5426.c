/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-07-22     liuduanfei   the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "touch.h"


#define DBG_TAG "ft5426"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static void ft54x6_write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t value)
{
    struct rt_i2c_msg msg = {0};
    rt_uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;

    msg.addr = (0x70>>1);
    msg.flags = RT_I2C_WR;
    msg.buf = buf;
    msg.len = 2;
   if (rt_i2c_transfer(bus, &msg, 1) != 1)
   {
       LOG_D("ft5426 write register error");
   }
}

static void ft54x6_read_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t *buf, rt_uint8_t len)
{
    struct rt_i2c_msg msg = {0};
    rt_uint8_t temp_reg;

    temp_reg = reg;

    msg.addr = (0x70>>1);
    msg.flags = RT_I2C_WR;
    msg.buf = &temp_reg;
    msg.len = 1;
    if (rt_i2c_transfer(bus, &msg, 1) != 1)
    {
        LOG_D("ft5426 write register error");
    }

    msg.addr = (0x70>>1);
    msg.flags = RT_I2C_RD;
    msg.buf = buf;
    msg.len = len;
    if (rt_i2c_transfer(bus, &msg, 1) != 1)
    {
        LOG_D("ft5426 read register error");
    }
}

static void ft5426_init(struct rt_i2c_bus_device *bus)
{
    ft54x6_write_reg(bus, 0x00, 0);
    ft54x6_write_reg(bus, 0xA4, 0);
    ft54x6_write_reg(bus, 0x80, 22);
    ft54x6_write_reg(bus, 0x88, 12);
}

static rt_size_t ft5426_readpoint(struct rt_touch_device *touch, void *data, rt_size_t touch_num)
{
    rt_uint8_t buf[4];
    struct rt_i2c_bus_device * i2c_bus = RT_NULL;

    static rt_uint8_t s_tp_down = 0;
    static uint16_t x_save, y_save;
    static rt_uint8_t s_count = 0;

    struct rt_touch_data *temp_data;

    temp_data = (struct rt_touch_data *)data;

    temp_data->event = RT_TOUCH_EVENT_NONE;
    temp_data->timestamp = rt_touch_get_ts();

    i2c_bus = rt_i2c_bus_device_find(touch->config.dev_name);
    if (i2c_bus == RT_NULL)
    {
        LOG_D("can not find i2c bus");
        return -RT_EIO;
    }

    ft54x6_read_reg(i2c_bus, 2, &buf[0], 1);

    if (buf[0] == 0)
    {
        if (s_tp_down == 1)
        {
            if (++s_count > 2)
            {
                s_count = 0;
                s_tp_down = 0;
                temp_data->event = RT_TOUCH_EVENT_UP;
                temp_data->timestamp = rt_touch_get_ts();
                temp_data->x_coordinate = x_save;
                temp_data->y_coordinate = y_save;

            }
        }
        return RT_EOK;
    }
    s_count = 0;

    ft54x6_read_reg(i2c_bus, 3, buf, touch_num * 4);

    temp_data->x_coordinate = ((buf[0]&0X0F)<<8)+buf[1];
    temp_data->y_coordinate = ((buf[2]&0X0F)<<8)+buf[3];

    temp_data->timestamp = rt_touch_get_ts();

    if (s_tp_down == 0)
    {
        s_tp_down = 1;
        temp_data->event = RT_TOUCH_EVENT_DOWN;
    }
    else
    {
        temp_data->event = RT_TOUCH_EVENT_MOVE;
    }
    x_save = temp_data->x_coordinate;
    y_save = temp_data->y_coordinate;

    return RT_EOK;
}
static rt_err_t ft5426_control(struct rt_touch_device *touch, int cmd, void *arg)
{
    struct rt_touch_info *info;

    switch(cmd)
    {
    case RT_TOUCH_CTRL_GET_ID:
        break;
    case RT_TOUCH_CTRL_GET_INFO:
        info = (struct rt_touch_info *)arg;
        info->point_num = touch->info.point_num;
        info->range_x = touch->info.range_x;
        info->range_y = touch->info.range_y;
        info->type = touch->info.type;
        info->vendor = touch->info.vendor;

        break;
    case RT_TOUCH_CTRL_SET_MODE:
        break;
    case RT_TOUCH_CTRL_SET_X_RANGE:
        break;
    case RT_TOUCH_CTRL_SET_Y_RANGE:
        break;
    case RT_TOUCH_CTRL_SET_X_TO_Y:
        break;
    case RT_TOUCH_CTRL_DISABLE_INT:
        break;
    case RT_TOUCH_CTRL_ENABLE_INT:
        break;
    default:
        break;
    }

    return RT_EOK;
}

const struct rt_touch_ops ops =
{
    ft5426_readpoint,
    ft5426_control,
};

struct rt_touch_info info =
{
    RT_TOUCH_TYPE_CAPACITANCE,
    RT_TOUCH_VENDOR_FT,
    1,
    480,
    800,
};

int rt_hw_ft5426_init(const char *name, struct rt_touch_config *cfg)
{
    rt_touch_t touch_device = RT_NULL;
    struct rt_i2c_bus_device * i2c_bus = RT_NULL;

    touch_device = (rt_touch_t)rt_calloc(1, sizeof(struct rt_touch_device));
    if (touch_device == RT_NULL)
    {
        return -RT_ENOMEM;
    }

    i2c_bus = rt_i2c_bus_device_find(cfg->dev_name);
    if (i2c_bus == RT_NULL)
    {
        return -RT_EIO;
    }

    ft5426_init(i2c_bus);

    rt_memcpy(&touch_device->config, cfg, sizeof(struct rt_touch_config));

    touch_device->info = info;
    touch_device->ops = &ops;

    if (rt_hw_touch_register(touch_device, name, RT_DEVICE_FLAG_RDONLY, RT_NULL) != RT_EOK)
    {
        return -RT_EIO;
    }

    return RT_EOK;
}
