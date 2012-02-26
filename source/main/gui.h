/* 
 * File:   gui.h
 * Author: cc
 *
 * Created on 26 février 2012, 21:29
 */

#ifndef GUI_H
#define	GUI_H

#ifdef	__cplusplus
extern "C" {
#endif

    struct HW_GPU_Config {
        int fps_limit;
        int hires_texture;
        int gte_accuracy;
    };

    struct SPU_Config {
        int enable_xa;
        int change_xa_speed;
        int irq_wait;
    };

    extern SPU_Config SpuConfig;
    extern HW_GPU_Config HwGpuConfig;


#ifdef	__cplusplus
}
#endif

#endif	/* GUI_H */

