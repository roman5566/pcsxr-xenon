#define _IN_TEXTURE

#include "stdafx.h"
#include "externals.h"
using namespace xegpu;
#include "texture.h"
#include "gpu.h"
#include "prim.h"
#include "GpuRenderer.h"
#include "texture_load.h"

// Sadly, there is also a second kind of texture cache needed, for "psx texture windows".
// Those are "repeated" textures, so a psx "texture window" needs to be put in
// a whole texture to use the GL_TEXTURE_WRAP_ features. This cache can get full very
// fast in games which are having an heavy "texture window" usage, like RRT4. As an
// alternative, this plugin can use the OGL "palette" extension on texture windows,
// if available. Nowadays also a fragment shader can easily be used to emulate
// texture wrapping in a texture atlas, so the main cache could hold the texture
// windows as well (that's what I am doing in the OGL2 plugin). But currently the
// OGL1 plugin is a "shader-free" zone, so heavy "texture window" games will cause
// much texture uploads.

/**
 * set data for small textures
 */
void XXsetTextureData(XenosSurface * surf,void * buffer){
    u8 * buf = (u8*)buffer;

    u8 * surfbuf;
    int j,i;
    surfbuf=(u8*)gpuRenderer.TextureLock(surf);
    for(j=0;j<surf->hpitch;++j)
        for(i=0;i<surf->wpitch;i+=surf->width*4)
            memcpy(&surfbuf[surf->wpitch*j+i],&buf[surf->width*(j%surf->height)*4],surf->width*4);

    gpuRenderer.TextureUnlock(surf);
}

GpuTex * XeDefineTextureWnd( GpuTex * gTexName) {
    int generate = (gTexName==NULL);
    if(generate==0){
        if( (TWin.Position.x1!=gTexName->width)|| (TWin.Position.y1!=gTexName->height)  )
        {
            generate = 1;
            // size changed
            printf("XeDefineTextureWnd regenerate\r\n");
            gpuRenderer.DestroyTexture(gTexName);
            gTexName=NULL;
        }
    }

    if(generate){
        gTexName = gpuRenderer.CreateTexture( TWin.Position.x1, TWin.Position.y1, FMT_A8R8G8B8);

        gTexName->u_addressing = XE_TEXADDR_WRAP;
        gTexName->v_addressing = XE_TEXADDR_WRAP;

        if (iFilterType && iFilterType < 3 && iHiResTextures != 2) {
            gTexName->use_filtering = XE_TEXF_LINEAR;
        } else {
            gTexName->use_filtering = XE_TEXF_POINT;
        }
    }
    return gTexName;
}
/*
void DefineTextureWnd(void) {
    if (gTexName == 0)
        gTexName = gpuRenderer.CreateTexture( TWin.Position.x1, TWin.Position.y1, FMT_A8R8G8B8);
#ifndef WIN32
    gTexName->u_addressing = XE_TEXADDR_WRAP;
    gTexName->v_addressing = XE_TEXADDR_WRAP;

    if (iFilterType && iFilterType < 3 && iHiResTextures != 2) {
        gTexName->use_filtering = XE_TEXF_LINEAR;
    } else {
        gTexName->use_filtering = XE_TEXF_POINT;
    }
#endif
    XXsetTextureData(gTexName, texturepart);
}
*/

////////////////////////////////////////////////////////////////////////
// tex window
////////////////////////////////////////////////////////////////////////
void XeLoadStretchWndTexturePage(GpuTex *wnd, int pageid, int mode, short cx, short cy) {
    TR;
    uint32_t start, row, column, j, sxh, sxm, ldx, ldy, ldxo, s;
    unsigned int palstart;
    uint32_t *px, *pa, *ta;
    unsigned char *cSRCPtr, *cOSRCPtr;
    unsigned short *wSRCPtr, *wOSRCPtr;
    uint32_t LineOffset;
    int pmult = pageid / 16;
    uint32_t(*LTCOL)(uint32_t);

    LTCOL = TCF[DrawSemiTrans];

    ldxo = TWin.Position.x1 - TWin.OPosition.x1;
    ldy = TWin.Position.y1 - TWin.OPosition.y1;

    pa = px = (uint32_t *) ubPaletteBuffer;
    ta = (uint32_t *) texturepart;
    palstart = cx + (cy * 1024);

    ubOpaqueDraw = 0;

    switch (mode) {
            //--------------------------------------------------//
            // 4bit texture load ..
        case 0:
            //-------------------

            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;
            // convert CLUT to 32bits .. and then use THAT as a lookup table

            wSRCPtr = psxVuw + palstart;
            for (row = 0; row < 16; row++)
                *px++ = LTCOL(ptr32(wSRCPtr++));

            sxm = g_x1 & 1;
            sxh = g_x1 >> 1;
            if (sxm) j = g_x1 + 1;
            else j = g_x1;
            cSRCPtr = psxVub + start + (2048 * g_y1) + sxh;
            for (column = g_y1; column <= g_y2; column++) {
                cOSRCPtr = cSRCPtr;
                ldx = ldxo;
                if (sxm) *ta++ = *(pa + ((*cSRCPtr++ >> 4) & 0xF));

                for (row = j; row <= g_x2 - ldxo; row++) {
                    s = *(pa + (*cSRCPtr & 0xF));
                    *ta++ = s;
                    if (ldx) {
                        *ta++ = s;
                        ldx--;
                    }
                    row++;
                    if (row <= g_x2 - ldxo) {
                        s = *(pa + ((*cSRCPtr >> 4) & 0xF));
                        *ta++ = s;
                        if (ldx) {
                            *ta++ = s;
                            ldx--;
                        }
                    }
                    cSRCPtr++;
                }
                if (ldy && column & 1) {
                    ldy--;
                    cSRCPtr = cOSRCPtr;
                } else cSRCPtr = psxVub + start + (2048 * (column + 1)) + sxh;
            }

            XXsetTextureData(wnd, texturepart);
            break;
            //--------------------------------------------------//
            // 8bit texture load ..
        case 1:
            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;

            // not using a lookup table here... speeds up smaller texture areas
            cSRCPtr = psxVub + start + (2048 * g_y1) + g_x1;
            LineOffset = 2048 - (g_x2 - g_x1 + 1) + ldxo;

            for (column = g_y1; column <= g_y2; column++) {
                cOSRCPtr = cSRCPtr;
                ldx = ldxo;
                for (row = g_x1; row <= g_x2 - ldxo; row++) {
                    s = LTCOL(ptr32(&psxVuw[palstart + *cSRCPtr++]));
                    *ta++ = s;
                    if (ldx) {
                        *ta++ = s;
                        ldx--;
                    }
                }
                if (ldy && column & 1) {
                    ldy--;
                    cSRCPtr = cOSRCPtr;
                } else cSRCPtr += LineOffset;
            }

            XXsetTextureData(wnd, texturepart);
            break;
            //--------------------------------------------------//
            // 16bit texture load ..
        case 2:
            start = ((pageid - 16 * pmult)*64) + 256 * 1024 * pmult;

            wSRCPtr = psxVuw + start + (1024 * g_y1) + g_x1;
            LineOffset = 1024 - (g_x2 - g_x1 + 1) + ldxo;

            for (column = g_y1; column <= g_y2; column++) {
                wOSRCPtr = wSRCPtr;
                ldx = ldxo;
                for (row = g_x1; row <= g_x2 - ldxo; row++) {
                    s = LTCOL(ptr32(wSRCPtr++));
                    *ta++ = s;
                    if (ldx) {
                        *ta++ = s;
                        ldx--;
                    }
                }
                if (ldy && column & 1) {
                    ldy--;
                    wSRCPtr = wOSRCPtr;
                } else wSRCPtr += LineOffset;
            }

            XXsetTextureData(wnd, texturepart);
            break;
            //--------------------------------------------------//
            // others are not possible !
    }
}


////////////////////////////////////////////////////////////////////////
// tex window: load simple
////////////////////////////////////////////////////////////////////////
void XeLoadWndTexturePage(GpuTex * wnd,int pageid, int mode, short cx, short cy) {
    uint32_t start, row, column, j, sxh, sxm;
    unsigned int palstart;
    uint32_t *px, *pa, *ta;
    unsigned char *cSRCPtr;
    unsigned short *wSRCPtr;
    uint32_t LineOffset;
    int pmult = pageid / 16;
    uint32_t(*LTCOL)(uint32_t);

    LTCOL = TCF[DrawSemiTrans];

    pa = px = (uint32_t *) ubPaletteBuffer;
    ta = (uint32_t *) texturepart;
    palstart = cx + (cy * 1024);

    ubOpaqueDraw = 0;

    switch (mode) {
            //--------------------------------------------------//
            // 4bit texture load ..
        case 0:
            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;

            // convert CLUT to 32bits .. and then use THAT as a lookup table

            wSRCPtr = psxVuw + palstart;
            for (row = 0; row < 16; row++)
                *px++ = LTCOL(ptr32(wSRCPtr++));

            sxm = g_x1 & 1;
            sxh = g_x1 >> 1;
            if (sxm) j = g_x1 + 1;
            else j = g_x1;
            cSRCPtr = psxVub + start + (2048 * g_y1) + sxh;
            for (column = g_y1; column <= g_y2; column++) {
                cSRCPtr = psxVub + start + (2048 * column) + sxh;

                if (sxm) *ta++ = *(pa + ((*cSRCPtr++ >> 4) & 0xF));

                for (row = j; row <= g_x2; row++) {
                    *ta++ = *(pa + (*cSRCPtr & 0xF));
                    row++;
                    if (row <= g_x2) *ta++ = *(pa + ((*cSRCPtr >> 4) & 0xF));
                    cSRCPtr++;
                }
            }

            XXsetTextureData(wnd, texturepart);
            break;
            //--------------------------------------------------//
            // 8bit texture load ..
        case 1:
            start = ((pageid - 16 * pmult)*128) + 256 * 2048 * pmult;

            // not using a lookup table here... speeds up smaller texture areas
            cSRCPtr = psxVub + start + (2048 * g_y1) + g_x1;
            LineOffset = 2048 - (g_x2 - g_x1 + 1);

            for (column = g_y1; column <= g_y2; column++) {
                for (row = g_x1; row <= g_x2; row++)
                    //*ta++ = LTCOL(psxVuw[palstart + *cSRCPtr++]);
                    *ta++ = LTCOL(ptr32(&psxVuw[palstart + *cSRCPtr++]));
                cSRCPtr += LineOffset;
            }

            XXsetTextureData(wnd, texturepart);
            break;
            //--------------------------------------------------//
            // 16bit texture load ..
        case 2:
            start = ((pageid - 16 * pmult)*64) + 256 * 1024 * pmult;

            wSRCPtr = psxVuw + start + (1024 * g_y1) + g_x1;
            LineOffset = 1024 - (g_x2 - g_x1 + 1);

            for (column = g_y1; column <= g_y2; column++) {
                for (row = g_x1; row <= g_x2; row++)
                    //*ta++ = LTCOL(*wSRCPtr++);
                    *ta++ = LTCOL(ptr32(wSRCPtr++));
                wSRCPtr += LineOffset;
            }

            XXsetTextureData(wnd, texturepart);
            break;
            //--------------------------------------------------//
            // others are not possible !
    }
}



////////////////////////////////////////////////////////////////////////
// tex window: main selecting, cache handler included
////////////////////////////////////////////////////////////////////////
GpuTex * LoadTextureWnd(int pageid, int TextureMode, uint32_t GivenClutId) {
    textureWndCacheEntry *ts, *tsx = NULL;
    int i;
    short cx, cy;
    EXLong npos;

    npos._3 = TWin.Position.x0;
    npos._2 = TWin.OPosition.x1;
    npos._1 = TWin.Position.y0;
    npos._0 = TWin.OPosition.y1;

    g_x1 = TWin.Position.x0;
    g_x2 = g_x1 + TWin.Position.x1 - 1;
    g_y1 = TWin.Position.y0;
    g_y2 = g_y1 + TWin.Position.y1 - 1;

    if (TextureMode == 2) {
        GivenClutId = 0;
        cx = cy = 0;
    } else {
        cx = ((GivenClutId << 4) & 0x3F0);
        cy = ((GivenClutId >> 6) & CLUTYMASK);
        GivenClutId = (GivenClutId & CLUTMASK) | (DrawSemiTrans << 30);

        // palette check sum
        {
            uint32_t l = 0, row;
            uint32_t *lSRCPtr = (uint32_t *) (psxVuw + cx + (cy * 1024));
            if (TextureMode == 1) for (row = 1; row < 129; row++) l += (GETLE32(lSRCPtr++) - 1) * row;
            else for (row = 1; row < 9; row++) l += (GETLE32(lSRCPtr++) - 1) << row;
            l = (l + HIWORD(l))&0x3fffL;
            GivenClutId |= (l << 16);
        }

    }

    ts = wcWndtexStore;

    for (i = 0; i < iMaxTexWnds; i++, ts++) {
        if (ts->used) {
            if (ts->pos.l == npos.l &&
                    ts->pageid == pageid &&
                    ts->textureMode == TextureMode) {
                if (ts->ClutID == GivenClutId) {
                    ubOpaqueDraw = ts->Opaque;
                    return ts->texname;
                }
            }
        } else tsx = ts;
    }

    if (!tsx) {
        if (iMaxTexWnds == iTexWndLimit) {
            tsx = wcWndtexStore + iTexWndTurn;
            iTexWndTurn++;
            if (iTexWndTurn == iTexWndLimit) iTexWndTurn = 0;
        } else {
            tsx = wcWndtexStore + iMaxTexWnds;
            iMaxTexWnds++;
        }
    }

    tsx->texname = XeDefineTextureWnd(tsx->texname);

    if (TWin.OPosition.y1 == TWin.Position.y1 &&
            TWin.OPosition.x1 == TWin.Position.x1) {

        XeLoadWndTexturePage(tsx->texname, pageid, TextureMode, cx, cy);
    }
    else {
        XeLoadStretchWndTexturePage(tsx->texname, pageid, TextureMode, cx, cy);
    }

    tsx->Opaque = ubOpaqueDraw;
    tsx->pos.l = npos.l;
    tsx->ClutID = GivenClutId;
    tsx->pageid = pageid;
    tsx->textureMode = TextureMode;
    tsx->used = 1;

    return tsx->texname;
}


////////////////////////////////////////////////////////////////////////
// Invalidate tex windows
////////////////////////////////////////////////////////////////////////
void InvalidateWndTextureArea(int X, int Y, int W, int H) {
    int i, px1, px2, py1, py2, iYM = 1;
    textureWndCacheEntry * tsw = wcWndtexStore;

    W += X - 1;
    H += Y - 1;
    if (X < 0) X = 0;
    if (X > 1023) X = 1023;
    if (W < 0) W = 0;
    if (W > 1023) W = 1023;
    if (Y < 0) Y = 0;
    if (Y > iGPUHeightMask) Y = iGPUHeightMask;
    if (H < 0) H = 0;
    if (H > iGPUHeightMask) H = iGPUHeightMask;
    W++;
    H++;

    if (iGPUHeight == 1024) iYM = 3;

    py1 = min(iYM, Y >> 8);
    py2 = min(iYM, H >> 8); // y: 0 or 1

    px1 = max(0, (X >> 6));
    px2 = min(15, (W >> 6));

    if (py1 == py2) {
        py1 = py1 << 4;
        px1 += py1;
        px2 += py1; // change to 0-31
        for (i = 0; i < iMaxTexWnds; i++, tsw++) {
            if (tsw->used) {

                //DumpExL(&tsw->pos);

                if (tsw->pageid >= px1 && tsw->pageid <= px2) {
                    tsw->used = 0;
                }
            }
        }
    } else {
        py1 = px1 + 16;
        py2 = px2 + 16;
        for (i = 0; i < iMaxTexWnds; i++, tsw++) {
            if (tsw->used) {

                //DumpExL(&tsw->pos);

                if ((tsw->pageid >= px1 && tsw->pageid <= px2) ||
                        (tsw->pageid >= py1 && tsw->pageid <= py2)) {
                    tsw->used = 0;
                }
            }
        }
    }

    // adjust tex window count
    tsw = wcWndtexStore + iMaxTexWnds - 1;
    while (iMaxTexWnds && !tsw->used) {
        iMaxTexWnds--;
        tsw--;
    }
}

