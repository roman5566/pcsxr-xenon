//#ifdef USE_GUI
#if 1
//#include <ogcsys.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <string>


//xenon stuff
#include <xetypes.h>
#include <xenon_smc/xenon_smc.h>
#include <xenon_soc/xenon_power.h>
#include <debug.h>
#include <usb/usbmain.h>
#include <input/input.h>
#include <ppc/timebase.h>
#include <time/time.h>
#include <usb/usbmain.h>
#include <console/console.h>
#include <usb/usbmain.h>
#include <xenon_soc/xenon_power.h>
#include <xenos/xenos.h>
#include <xenos/xe.h>
#include <xenon_sound/sound.h>
#include <xenos/edram.h>
#include <libntfs/ntfs.h>
#include <libext2fs/ext2.h>

#include <newlib/malloc_lock.h>

//gui stuff
#include "libwiigui/gui.h"
#include "input.h"
#include "filelist.h"
#include "g_filebrowser.h"
#include "w_input.h"
#include "gui_debug.h"
#include "gui_romlist.h"



// pcsxr stuff
#define PCSXR_VERSION   "0.62"
#define PCSXR_NAME      "PCSXR Xenon"
#define PCSXR_APP_NAME  PCSXR_NAME " v" PCSXR_VERSION
#include "psxcommon.h"
#include "config.h"
#include "r3000a.h"
#include "debug.h"
#include "sio.h"
#include "misc.h"
#include "gamecube_plugins.h"

#include "gui.h"

extern "C" {
    void useSoftGpu();
    void useHwGpu();
}

SPU_Config SpuConfig;
HW_GPU_Config HwGpuConfig;
char ROMFilename[1024];
char foldername[1024];

enum {
    MENU_EXIT = -1,
    MENU_NONE,
    MENU_MAIN,
    MENU_OPTIONS,
    MENU_CHEATS,
    MENU_SAVE,
    MENU_LOAD,
    MENU_SETTINGS,
    MENU_SETTINGS_FILE,
    MENU_IN_GAME,
    MENU_BROWSE_DEVICE,
    MENU_GAME_LOAD,
    MENU_EMULATION,
    MENU_SPU,
    MENU_GPU,
};


static int pcsxr_running = 0;
static int pcsxr_exit_asked = 0;

#define THREAD_SLEEP 100

static GuiImageData * pointer[4];
static GuiImage * bgImg = NULL;
//static GuiSound * bgMusic = NULL;
static GuiWindow * mainWindow = NULL;
static bool guiHalt = true;

static void TH_UGUI();

GXColor ColorGrey = {104, 104, 104, 255};
GXColor ColorGrey2 = {49, 49, 49, 255};
GXColor ColorWhite = {255, 255, 255, 255};

// emulator option
OptionList options;

/****************************************************************************
 * ResumeGui
 *
 * Signals the GUI thread to start, and resumes the thread. This is called
 * after finishing the removal/insertion of new elements, and after initial
 * GUI setup.
 ***************************************************************************/
static void
_ResumeGui() {
    guiHalt = false;
    udelay(THREAD_SLEEP);
}

/****************************************************************************
 * HaltGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
static void
_HaltGui() {
    guiHalt = true;
    udelay(THREAD_SLEEP);
}
//
#define ResumeGui(){TR;_ResumeGui();}
#define HaltGui(){TR;_HaltGui();}

extern "C" void GuiScreenCapture();

void GuiScreenCaps() {
    HaltGui();
    // wait for vblank
    while (!Xe_IsVBlank(g_pVideoDevice));
    GuiScreenCapture();
    ResumeGui();
}

/****************************************************************************
 * WindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice
 ***************************************************************************/
int WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label) {
    int choice = -1;
    //    GuiWindow promptWindow(448, 288);
    GuiWindow promptWindow(640, 360);
    promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    promptWindow.SetPosition(0, -10);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(xenon_button_png);
    GuiImageData btnOutlineOver(xenon_button_over_png);
    GuiTrigger trigA;

    //	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigA.SetSimpleTrigger(-1, 0, PAD_BUTTON_A);

    //    GuiImageData dialogBox(dialogue_box_png);
    GuiImageData dialogBox(xenon_popup_png);
    GuiImage dialogBoxImg(&dialogBox);

    GuiText titleTxt(title, 26, ColorGrey);
    titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    titleTxt.SetPosition(0, 40);

    GuiText msgTxt(msg, 22, ColorGrey2);
    msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
    msgTxt.SetPosition(0, -20);
    msgTxt.SetWrap(true, 600);

    GuiText btn1Txt(btn1Label, 22, ColorGrey);
    GuiImage btn1Img(&btnOutline);
    GuiImage btn1ImgOver(&btnOutlineOver);
    GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());

    if (btn2Label) {
        btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
        btn1.SetPosition(20, -25);
    } else {
        btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
        btn1.SetPosition(0, -25);
    }

    btn1.SetLabel(&btn1Txt);
    btn1.SetImage(&btn1Img);
    btn1.SetImageOver(&btn1ImgOver);
    btn1.SetSoundOver(&btnSoundOver);
    btn1.SetTrigger(&trigA);
    btn1.SetState(STATE_SELECTED);
    btn1.SetEffectGrow();

    GuiText btn2Txt(btn2Label, 22, ColorGrey);
    GuiImage btn2Img(&btnOutline);
    GuiImage btn2ImgOver(&btnOutlineOver);
    GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
    btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    btn2.SetPosition(-20, -25);
    btn2.SetLabel(&btn2Txt);
    btn2.SetImage(&btn2Img);
    btn2.SetImageOver(&btn2ImgOver);
    btn2.SetSoundOver(&btnSoundOver);
    btn2.SetTrigger(&trigA);
    btn2.SetEffectGrow();

    promptWindow.Append(&dialogBoxImg);

    promptWindow.Append(&titleTxt);
    promptWindow.Append(&msgTxt);
    promptWindow.Append(&btn1);

    if (btn2Label)
        promptWindow.Append(&btn2);

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    HaltGui();

    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&promptWindow);
    mainWindow->ChangeFocus(&promptWindow);
    ResumeGui();

    while (choice == -1) {
        udelay(THREAD_SLEEP);

        TH_UGUI();

        if (btn1.GetState() == STATE_CLICKED)
            choice = 1;
        else if (btn2.GetState() == STATE_CLICKED)
            choice = 0;
    }

    promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while (promptWindow.GetEffect() > 0) {
        udelay(THREAD_SLEEP);

        TH_UGUI();
    }
    HaltGui();
    mainWindow->Remove(&promptWindow);
    mainWindow->SetState(STATE_DEFAULT);

    ResumeGui();

    HaltGui();
    return choice;
}

void ErrorPrompt(const char *msg) {
    WindowPrompt("Error", msg, "OK", NULL);
}

void InfoPrompt(const char *msg) {
    WindowPrompt("Information", msg, "OK", NULL);
}

/****************************************************************************
 * UpdateGUI
 *
 * Primary thread to allow GUI to respond to state changes, and draws GUI
 ***************************************************************************/
void TH_UGUI() {
    int i;
    UpdatePads();
    mainWindow->Draw();
    Menu_Render();
    for (i = 0; i < 4; i++) {
        mainWindow->Update(&userInput[i]);
    }

#if 0
    if (ExitRequested) {
        for (i = 0; i <= 255; i += 15) {
            mainWindow->Draw();

            Menu_DrawRectangle(0, 0, screenwidth, screenheight, (XeColor) {
                i, 0, 0, 0
            }, 1);
            Menu_Render();
        }
        ExitApp();
    }
#endif
}

static void * UpdateGUI() {
    while (1) {
        udelay(THREAD_SLEEP);
        if (guiHalt) {

        } else {
            TH_UGUI();
        }
    }
    return NULL;
}

/****************************************************************************
 * InitGUIThread
 *
 * Startup GUI threads
 ***************************************************************************/
static unsigned char gui_thread_stack[0x10000];

void InitGUIThreads() {
    // xenon_run_thread_task(5, &gui_thread_stack[sizeof (gui_thread_stack) - 0x100], (void*) UpdateGUI);
}

/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
static void OnScreenKeyboard(char * var, u16 maxlen) {
    int save = -1;

    GuiKeyboard keyboard(var, maxlen);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(xenon_button_png);
    GuiImageData btnOutlineOver(xenon_button_over_png);
    GuiTrigger trigA;
    //	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigA.SetSimpleTrigger(-1, 0, PAD_BUTTON_A);

    GuiText okBtnTxt("OK", 22, (GXColor) {
        0, 0, 0, 255
    });
    GuiImage okBtnImg(&btnOutline);
    GuiImage okBtnImgOver(&btnOutlineOver);
    GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

    okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    okBtn.SetPosition(25, -25);

    okBtn.SetLabel(&okBtnTxt);
    okBtn.SetImage(&okBtnImg);
    okBtn.SetImageOver(&okBtnImgOver);
    okBtn.SetSoundOver(&btnSoundOver);
    okBtn.SetTrigger(&trigA);
    okBtn.SetEffectGrow();

    GuiText cancelBtnTxt("Cancel", 22, (GXColor) {
        0, 0, 0, 255
    });
    GuiImage cancelBtnImg(&btnOutline);
    GuiImage cancelBtnImgOver(&btnOutlineOver);
    GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
    cancelBtn.SetPosition(-25, -25);
    cancelBtn.SetLabel(&cancelBtnTxt);
    cancelBtn.SetImage(&cancelBtnImg);
    cancelBtn.SetImageOver(&cancelBtnImgOver);
    cancelBtn.SetSoundOver(&btnSoundOver);
    cancelBtn.SetTrigger(&trigA);
    cancelBtn.SetEffectGrow();

    keyboard.Append(&okBtn);
    keyboard.Append(&cancelBtn);

    HaltGui();
    mainWindow->SetState(STATE_DISABLED);
    mainWindow->Append(&keyboard);
    mainWindow->ChangeFocus(&keyboard);
    ResumeGui();

    while (save == -1) {
        udelay(THREAD_SLEEP);

        TH_UGUI();

        if (okBtn.GetState() == STATE_CLICKED)
            save = 1;
        else if (cancelBtn.GetState() == STATE_CLICKED)
            save = 0;
    }

    if (save) {
        snprintf(var, maxlen, "%s", keyboard.kbtextstr);
    }

    HaltGui();
    mainWindow->Remove(&keyboard);
    mainWindow->SetState(STATE_DEFAULT);
    ResumeGui();
}

static int progress_done = 0;
static int progress_total = 0;
static char progress_str[200];

void ShowProgress(const char *msg, int done, int total) {
    progress_done = done;
    progress_total = total;
    snprintf(progress_str, 200, "%s %d/%d", msg, done, total);

    TH_UGUI();
}

static void ProgressUpdateCallback(void * e) {
    GuiText * _this = (GuiText *) e;
    _this->SetText(progress_str);
}

void SetIso(const char * fname) {
    FILE *fd = fopen(fname, "rb");
    if (fd == NULL) {
        printf("Error loading %s\r\n", fname);
        return;
    }
    uint8_t header[0x10];
    int n = fread(header, 0x10, 1, fd);

    if (header[0] == 0x78 && header[1] == 0xDA) {
        printf("Use CDRCIMG for  %s\r\n", fname);
        strcpy(Config.Cdr, "CDRCIMG");
        cdrcimg_set_fname(fname);
    } else {
        SetIsoFile(fname);
    }
    fclose(fd);
}

void makeRomName(char * fname, char * dest) {
    char * end = strrchr(fname, '.');
    strncpy(dest, fname, end - fname);
}

static int pcsxr_run(void) {
    char cdfile[2048];

    sprintf(foldername, "%s/", browser.dir);
    makeRomName(browserList[browser.selIndex].filename, ROMFilename);
    sprintf(cdfile, "%s/%s/%s", rootdir, browser.dir, browserList[browser.selIndex].filename);

    printf("ROMFilename : %s\r\n", ROMFilename);

    strcpy(Config.Net, "Disabled");
    strcpy(Config.Cdr, "CDR");
    strcpy(Config.Gpu, "GPU");
    strcpy(Config.Spu, "SPU");
    strcpy(Config.Pad1, "PAD1");
    strcpy(Config.Pad2, "PAD2");

    strcpy(Config.BiosDir, "uda:/pcsxr/bios");
    strcpy(Config.Bios, "scph7502.bin");

    strcpy(Config.Mcd1, "uda:/pcsxr/memcards/card1.mcd");
    strcpy(Config.Mcd2, "uda:/pcsxr/memcards/card2.mcd");

    pcsxr_running = 0;

    SetIso(cdfile);
    if (LoadPlugins() == 0) {
        if (OpenPlugins() == 0) {
            if (SysInit() == -1) {
                ErrorPrompt("SysInit() Error!\n");
                return -1;
            }

            SysReset();
            // Check for hle ...
            if (Config.HLE == 1) {
                ErrorPrompt("Can't continue ... bios not found ...");
                return -1;
            }

            int ret = CheckCdrom();
            if (CheckCdrom() != 0) {
                ErrorPrompt("Can't continue ... invalide cd-image detected ...");
                return -1;
            }
            ret = LoadCdrom();
            if (ret != 0) {
                ErrorPrompt("Can't continue ... no executable found ...");
                return -1;
            }
            pcsxr_running = 1;
            return 1;
        }
    }

    return -1;
}

/****************************************************************************
 * MenuBrowseDevice
 ***************************************************************************/
static int MenuBrowseDevice() {
    char title[100];
    int i;

    TR;
    ShutoffRumble();

    // populate initial directory listing
    if (BrowseDevice() <= 0) {
        int choice = WindowPrompt(
                "Error",
                "Unable to display files on selected load device.",
                "Retry",
                "Check Settings");

        if (choice)
            return MENU_BROWSE_DEVICE;
        else
            return MENU_SETTINGS;
    }

    int menu = MENU_NONE;

    sprintf(title, "%s - Load Game", PCSXR_APP_NAME);

    GuiText titleTxt(PCSXR_APP_NAME" - Load Game", 28, ColorGrey);
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    titleTxt.SetPosition(50, 50);

    GuiTrigger trigA;
    //	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigA.SetSimpleTrigger(-1, 0, PAD_BUTTON_A);

    GuiFileBrowser fileBrowser(1080, 496);
    fileBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    fileBrowser.SetPosition(0, 100);

    GuiImageData btnOutline(xenon_button_png);
    GuiImageData btnOutlineOver(xenon_button_over_png);

    GuiText backBtnTxt("Go Back", 22, ColorGrey2);
    GuiImage backBtnImg(&btnOutline);
    GuiImage backBtnImgOver(&btnOutlineOver);
    GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    backBtn.SetPosition(100, -35);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetImage(&backBtnImg);
    backBtn.SetImageOver(&backBtnImgOver);
    backBtn.SetTrigger(&trigA);
    backBtn.SetEffectGrow();

    GuiWindow xenon_buttonWindow(screenwidth, screenheight);
    xenon_buttonWindow.Append(&backBtn);

    HaltGui();
    mainWindow->Append(&titleTxt);
    mainWindow->Append(&fileBrowser);
    mainWindow->Append(&xenon_buttonWindow);
    ResumeGui();

    while (menu == MENU_NONE) {
        TH_UGUI();
        usleep(THREAD_SLEEP);
        // update file browser based on arrow xenon_buttons
        // set MENU_EXIT if A xenon_button pressed on a file
        for (i = 0; i < FILE_PAGESIZE; i++) {
            if (fileBrowser.fileList[i]->GetState() == STATE_CLICKED) {
                fileBrowser.fileList[i]->ResetState();
                // check corresponding browser entry
                if (browserList[browser.selIndex].isdir) {
                    if (BrowserChangeFolder()) {
                        fileBrowser.ResetState();
                        fileBrowser.fileList[0]->SetState(STATE_SELECTED);
                        fileBrowser.TriggerUpdate();
                    } else {
                        menu = MENU_BROWSE_DEVICE;
                        break;
                    }
                } else {
                    ShutoffRumble();
                    fileBrowser.ResetState();
                    //mainWindow->SetState(STATE_DISABLED);

                    if(pcsxr_run()==1)
                        menu = MENU_EMULATION;
                    //mainWindow->SetState(STATE_DEFAULT);
                }
            }
        }
        if (backBtn.GetState() == STATE_CLICKED)
            menu = MENU_SETTINGS;
    }
    HaltGui();
    mainWindow->Remove(&titleTxt);
    mainWindow->Remove(&xenon_buttonWindow);
    mainWindow->Remove(&fileBrowser);
    return menu;
}


struct controller_data_s ctrl;
struct controller_data_s old_ctrl;

 extern "C" void systemPoll() {
    get_controller_data(&ctrl, 0);
    static int reset_time = 0;
    if (ctrl.logo) {
        // exit(0);
        if (old_ctrl.logo) {
            reset_time++;
            if (reset_time > 2) {
                if (ctrl.select) {
                    GuiScreenCaps();
                } else {
                    pcsxr_exit_asked = 1;
                }
            }
        } else {
            reset_time = 0;
        }
    }
    old_ctrl = ctrl;

    if (pcsxr_exit_asked) {
        pcsxr_exit_asked = 0;
        psxCpu->Shutdown();
    }
}

void gui_vsync() {
    TH_UGUI();
}

static int MenuMain() {
    pcsxr_running = 0;

    int menu = MENU_NONE;

    GuiText titleTxt(PCSXR_APP_NAME, 28, ColorGrey);
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    titleTxt.SetPosition(50, 50);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(xenon_button_png);
    GuiImageData btnOutlineOver(xenon_button_over_png);
    GuiImageData btnLargeOutline(xenon_button_large_png);
    GuiImageData btnLargeOutlineOver(xenon_button_large_over_png);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    //    trigA.SetSimpleTrigger(-1, 0, PAD_BUTTON_A);
    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_LOGO);
    //    trigHome.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_LOGO);

    GuiText fileBtnTxt("Load Game", 18, ColorGrey2);
    fileBtnTxt.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    GuiImage fileBtnImg(&btnLargeOutline);
    GuiImage fileBtnImgOver(&btnLargeOutlineOver);
    GuiButton fileBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    fileBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    fileBtn.SetPosition(50, 120);
    fileBtn.SetLabel(&fileBtnTxt);
    fileBtn.SetImage(&fileBtnImg);
    fileBtn.SetImageOver(&fileBtnImgOver);
    fileBtn.SetSoundOver(&btnSoundOver);
    fileBtn.SetTrigger(&trigA);
    fileBtn.SetEffectGrow();

    GuiText optionBtnTxt("Options", 18, ColorGrey2);
    optionBtnTxt.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    GuiImage optionBtnImg(&btnLargeOutline);
    GuiImage optionBtnImgOver(&btnLargeOutlineOver);
    GuiButton optionBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    optionBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    optionBtn.SetPosition(250, 120);
    optionBtn.SetLabel(&optionBtnTxt);
    optionBtn.SetImage(&optionBtnImg);
    optionBtn.SetImageOver(&optionBtnImgOver);
    optionBtn.SetSoundOver(&btnSoundOver);
    optionBtn.SetTrigger(&trigA);
    optionBtn.SetEffectGrow();

    GuiText savingBtnTxt1("Load / Save", 18, ColorGrey2);
    savingBtnTxt1.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    GuiImage savingBtnImg(&btnLargeOutline);
    GuiImage savingBtnImgOver(&btnLargeOutlineOver);
    GuiButton savingBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    savingBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    savingBtn.SetPosition(-50, 120);
    savingBtn.SetLabel(&savingBtnTxt1);
    savingBtn.SetImage(&savingBtnImg);
    savingBtn.SetImageOver(&savingBtnImgOver);
    savingBtn.SetSoundOver(&btnSoundOver);
    savingBtn.SetTrigger(&trigA);
    savingBtn.SetEffectGrow();

    GuiText cheatsBtnTxt("Cheats", 18, ColorGrey2);
    cheatsBtnTxt.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    GuiImage cheatsBtnImg(&btnLargeOutline);
    GuiImage cheatsBtnImgOver(&btnLargeOutlineOver);
    GuiButton cheatsBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    cheatsBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    cheatsBtn.SetPosition(0, 250);
    cheatsBtn.SetLabel(&cheatsBtnTxt);
    cheatsBtn.SetImage(&cheatsBtnImg);
    cheatsBtn.SetImageOver(&cheatsBtnImgOver);
    cheatsBtn.SetSoundOver(&btnSoundOver);
    cheatsBtn.SetTrigger(&trigA);
    cheatsBtn.SetEffectGrow();

    GuiText exitBtnTxt("Exit to XELL", 18, ColorGrey2);
    GuiImage exitBtnImg(&btnOutline);
    GuiImage exitBtnImgOver(&btnOutlineOver);
    GuiButton exitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    exitBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    exitBtn.SetPosition(50, -35);
    exitBtn.SetLabel(&exitBtnTxt);
    exitBtn.SetImage(&exitBtnImg);
    exitBtn.SetImageOver(&exitBtnImgOver);
    exitBtn.SetSoundOver(&btnSoundOver);
    exitBtn.SetTrigger(&trigA);
    exitBtn.SetTrigger(&trigHome);
    exitBtn.SetEffectGrow();


    GuiText shutdown_btnTxt("Shutdown", 18, ColorGrey2);
    GuiImage shutdown_btnImg(&btnOutline);
    GuiImage shutdown_btnImgOver(&btnOutlineOver);
    GuiButton shutdown_btn(btnOutline.GetWidth(), btnOutline.GetHeight());
    shutdown_btn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    shutdown_btn.SetPosition(250, -35);
    shutdown_btn.SetLabel(&shutdown_btnTxt);
    shutdown_btn.SetImage(&shutdown_btnImg);
    shutdown_btn.SetImageOver(&shutdown_btnImgOver);
    shutdown_btn.SetSoundOver(&btnSoundOver);
    shutdown_btn.SetTrigger(&trigA);
    shutdown_btn.SetTrigger(&trigHome);
    shutdown_btn.SetEffectGrow();

    GuiText about_btnTxt("About", 18, ColorGrey2);
    GuiImage about_btnImg(&btnOutline);
    GuiImage about_btnImgOver(&btnOutlineOver);
    GuiButton about_btn(btnOutline.GetWidth(), btnOutline.GetHeight());
    about_btn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    about_btn.SetPosition(450, -35);
    about_btn.SetLabel(&about_btnTxt);
    about_btn.SetImage(&about_btnImg);
    about_btn.SetImageOver(&about_btnImgOver);
    about_btn.SetSoundOver(&btnSoundOver);
    about_btn.SetTrigger(&trigA);
    about_btn.SetTrigger(&trigHome);
    about_btn.SetEffectGrow();

    GuiText configspuBtnTxt("SPU Config", 18, ColorGrey2);
    configspuBtnTxt.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    GuiImage configspuBtnImg(&btnLargeOutline);
    GuiImage configspuBtnImgOver(&btnLargeOutlineOver);
    GuiButton configspuBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    configspuBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    configspuBtn.SetPosition(450, 120);
    configspuBtn.SetLabel(&configspuBtnTxt);
    configspuBtn.SetImage(&configspuBtnImg);
    configspuBtn.SetImageOver(&configspuBtnImgOver);
    configspuBtn.SetSoundOver(&btnSoundOver);
    configspuBtn.SetTrigger(&trigA);
    configspuBtn.SetEffectGrow();

    GuiText configgpuBtnTxt("GPU Config", 18, ColorGrey2);
    configgpuBtnTxt.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    GuiImage configgpuBtnImg(&btnLargeOutline);
    GuiImage configgpuBtnImgOver(&btnLargeOutlineOver);
    GuiButton configgpuBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    configgpuBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    configgpuBtn.SetPosition(650, 120);
    configgpuBtn.SetLabel(&configgpuBtnTxt);
    configgpuBtn.SetImage(&configgpuBtnImg);
    configgpuBtn.SetImageOver(&configgpuBtnImgOver);
    configgpuBtn.SetSoundOver(&btnSoundOver);
    configgpuBtn.SetTrigger(&trigA);
    configgpuBtn.SetEffectGrow();

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&titleTxt);
    w.Append(&fileBtn);
    w.Append(&optionBtn);
    w.Append(&shutdown_btn);
    //    w.Append(&savingBtn);
    // w.Append(&cheatsBtn); // unused
    w.Append(&exitBtn);

    w.Append(&configgpuBtn);
    w.Append(&configspuBtn);

    mainWindow->Append(&w);

    ResumeGui();

    while (menu == MENU_NONE) {
        TH_UGUI();
        usleep(THREAD_SLEEP);

        if (fileBtn.GetState() == STATE_CLICKED) {
            menu = MENU_BROWSE_DEVICE;
        } else if (optionBtn.GetState() == STATE_CLICKED) {
            menu = MENU_OPTIONS;
        } else if (savingBtn.GetState() == STATE_CLICKED) {
            menu = MENU_SAVE;
        } else if (optionBtn.GetState() == STATE_CLICKED) {
            menu = MENU_OPTIONS;
        } else if (configgpuBtn.GetState() == STATE_CLICKED) {
            menu = MENU_GPU;
        } else if (configspuBtn.GetState() == STATE_CLICKED) {
            menu = MENU_SPU;
        } else if (exitBtn.GetState() == STATE_CLICKED) {

            exitBtn.ResetState();

            int choice = WindowPrompt(
                    "Exit",
                    "Are you sure that you want to exit?",
                    "Yes",
                    "No");
            if (choice == 1) {
                exit(0);
            }

        } else if (shutdown_btn.GetState() == STATE_CLICKED) {
            shutdown_btn.ResetState();

            int choice = WindowPrompt(
                    "Shutdown",
                    "Are you sure that you want to shutdown?",
                    "Yes",
                    "No");
            if (choice == 1) {
                xenon_smc_power_shutdown();
            }
        } else if (about_btn.GetState() == STATE_CLICKED) {
            about_btn.ResetState();

            InfoPrompt(
                    PCSXR_NAME
                    );

        }
    }

    HaltGui();
    mainWindow->Remove(&w);
    return menu;
}

static int MenuConfig() {
    int menu = MENU_NONE;
    int ret;
    int i = 0;
    bool firstRun = true;

    int gpu_plugin = 0;
    TR;

    //settings_load();

    OptionList options;
    sprintf(options.name[i++], "Gpu plugin");
    sprintf(options.name[i++], "Cpu");
    sprintf(options.name[i++], "Xa Decoding");
    sprintf(options.name[i++], "Sio Irq");
    sprintf(options.name[i++], "Black and white movies");
    sprintf(options.name[i++], "Auto Detect Region");
    sprintf(options.name[i++], "Cd Audio");
    //    sprintf(options.name[i++], "HLE");
    sprintf(options.name[i++], "Slow Boot");
    //    sprintf(options.name[i++], "Debugger");
    //    sprintf(options.name[i++], "Console Ouput");
    sprintf(options.name[i++], "Spu Irq");
    sprintf(options.name[i++], "Parasite Eve 2, Vandal Hearts 1/2 Fix");
    //    sprintf(options.name[i++], "Use network");
    sprintf(options.name[i++], "InuYasha Sengoku Battle Fix");
    options.length = i;

    for (i = 0; i < options.length; i++)
        options.value[i][0] = 0;

    GuiText titleTxt(PCSXR_APP_NAME" - Options", 28, ColorGrey);
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    titleTxt.SetPosition(50, 50);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(xenon_button_png);
    GuiImageData btnOutlineOver(xenon_button_over_png);

    GuiTrigger trigA;
    //	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigA.SetSimpleTrigger(-1, 0, PAD_BUTTON_A);

    GuiText backBtnTxt("Go Back", 22, ColorGrey2);
    GuiImage backBtnImg(&btnOutline);
    GuiImage backBtnImgOver(&btnOutlineOver);
    GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    backBtn.SetPosition(100, -35);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetImage(&backBtnImg);
    backBtn.SetImageOver(&backBtnImgOver);
    backBtn.SetSoundOver(&btnSoundOver);
    backBtn.SetTrigger(&trigA);
    backBtn.SetEffectGrow();

    GuiOptionBrowser optionBrowser(1080, 496, &options);
    optionBrowser.SetPosition(0, 108);
    optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    optionBrowser.SetCol2Position(500);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&backBtn);
    mainWindow->Append(&optionBrowser);
    mainWindow->Append(&w);
    mainWindow->Append(&titleTxt);
    ResumeGui();

    while (menu == MENU_NONE) {
        TH_UGUI();
        usleep(THREAD_SLEEP);

        ret = optionBrowser.GetClickedOption();

        switch (ret) {
            case 0:
                gpu_plugin++;
                if (gpu_plugin > 1)
                    gpu_plugin = 0;
                break;
            case 1:
                Config.Cpu++;
                if (Config.Cpu > CPU_INTERPRETER)
                    Config.Cpu = 0;
                break;
            case 2:
                Config.Xa++;
                if (Config.Xa > 1)
                    Config.Xa = 0;
                break;
            case 3:
                Config.Sio++;
                if (Config.Sio > 1)
                    Config.Sio = 0;
                break;
            case 4:
                Config.Mdec++;
                if (Config.Mdec > 1)
                    Config.Mdec = 0;
                break;
            case 5:
                Config.PsxAuto++;
                if (Config.PsxAuto > 1)
                    Config.PsxAuto = 0;
                break;
            case 6:
                Config.Cdda++;
                if (Config.Cdda > 1)
                    Config.Cdda = 0;
                break;
                //            case 6:
                //                Config.HLE++;
                //                if (Config.HLE > 1)
                //                    Config.HLE = 0;
                //                break;
            case 7:
                Config.SlowBoot++;
                if (Config.SlowBoot > 1)
                    Config.SlowBoot = 0;
                break;
                //            case 8:
                //                Config.Debug++;
                //                if (Config.Debug > 1)
                //                    Config.Debug = 0;
                //                break;
                //            case 9:
                //                Config.PsxOut++;
                //                if (Config.PsxOut > 1)
                //                    Config.PsxOut = 0;
                //                break;
            case 8:
                Config.SpuIrq++;
                if (Config.SpuIrq > 1)
                    Config.SpuIrq = 0;
                break;
            case 9:
                Config.RCntFix++;
                if (Config.RCntFix > 1)
                    Config.RCntFix = 0;
                break;
                //            case 12:
                //                Config.UseNet++;
                //                if (Config.UseNet > 1)
                //                    Config.UseNet = 0;
                //                break;
            case 10:
                Config.VSyncWA++;
                if (Config.VSyncWA > 1)
                    Config.VSyncWA = 0;
                break;
        }

        if (ret >= 0 || firstRun) {
            firstRun = false;
            int j = 0;
            if (gpu_plugin)
                sprintf(options.value[j], "Soft");
            else
                sprintf(options.value[j], "Hw");
            j++;
            if (Config.Cpu)
                sprintf(options.value[j], "Interpreter");
            else
                sprintf(options.value[j], "Dynarec");

#define enabled_disabled(x) j++; \
            if (x) \
                sprintf(options.value[j], "Enabled"); \
            else \
                sprintf(options.value[j], "Disabled");
            \

#define disabled_enabled(x) j++; \
            if (x) \
                sprintf(options.value[j], "Disabled"); \
            else \
                sprintf(options.value[j], "Enabled");
            \

            disabled_enabled(Config.Xa);
            enabled_disabled(Config.Sio);
            enabled_disabled(Config.Mdec);
            enabled_disabled(Config.PsxAuto);
            disabled_enabled(Config.Cdda);
            enabled_disabled(Config.HLE);
            enabled_disabled(Config.SlowBoot);
            enabled_disabled(Config.Debug);
            enabled_disabled(Config.PsxOut);
            enabled_disabled(Config.SpuIrq);
            enabled_disabled(Config.RCntFix);
            enabled_disabled(Config.UseNet);
            enabled_disabled(Config.VSyncWA);

            optionBrowser.TriggerUpdate();
        }

        if (backBtn.GetState() == STATE_CLICKED) {
            menu = MENU_MAIN;
        }

    }
    HaltGui();


    // apply plugn selection
    if (gpu_plugin) {
        useSoftGpu();
    } else {
        useHwGpu();
    }

    mainWindow->Remove(&optionBrowser);
    mainWindow->Remove(&w);
    mainWindow->Remove(&titleTxt);
    return menu;
}

static int MenuConfigSPU() {
    int menu = MENU_NONE;
    int ret;
    int i = 0;
    bool firstRun = true;

    TR;

    OptionList options;
    sprintf(options.name[i++], "XA playing");
    sprintf(options.name[i++], "Change XA Speed");
    sprintf(options.name[i++], "IRQ Wait");
    options.length = i;

    for (i = 0; i < options.length; i++)
        options.value[i][0] = 0;

    GuiText titleTxt(PCSXR_APP_NAME" - SPU Options", 28, ColorGrey);
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    titleTxt.SetPosition(50, 50);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(xenon_button_png);
    GuiImageData btnOutlineOver(xenon_button_over_png);

    GuiTrigger trigA;
    //	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigA.SetSimpleTrigger(-1, 0, PAD_BUTTON_A);

    GuiText backBtnTxt("Go Back", 22, ColorGrey2);
    GuiImage backBtnImg(&btnOutline);
    GuiImage backBtnImgOver(&btnOutlineOver);
    GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    backBtn.SetPosition(100, -35);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetImage(&backBtnImg);
    backBtn.SetImageOver(&backBtnImgOver);
    backBtn.SetSoundOver(&btnSoundOver);
    backBtn.SetTrigger(&trigA);
    backBtn.SetEffectGrow();

    GuiOptionBrowser optionBrowser(1080, 496, &options);
    optionBrowser.SetPosition(0, 108);
    optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    optionBrowser.SetCol2Position(500);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&backBtn);
    mainWindow->Append(&optionBrowser);
    mainWindow->Append(&w);
    mainWindow->Append(&titleTxt);
    ResumeGui();

    while (menu == MENU_NONE) {
        TH_UGUI();
        usleep(THREAD_SLEEP);

        ret = optionBrowser.GetClickedOption();

        switch (ret) {
            case 0:
                SpuConfig.enable_xa++;
                if (SpuConfig.enable_xa > 1)
                    SpuConfig.enable_xa = 0;
                break;
            case 1:
                SpuConfig.change_xa_speed++;
                if (SpuConfig.change_xa_speed > 1)
                    SpuConfig.change_xa_speed = 0;
                break;
            case 2:
                SpuConfig.irq_wait++;
                if (SpuConfig.irq_wait > 1)
                    SpuConfig.irq_wait = 0;
                break;
        }

        if (ret >= 0 || firstRun) {
            firstRun = false;

            if (SpuConfig.enable_xa)
                sprintf(options.value[0], "Enabled");
            else
                sprintf(options.value[0], "Disabled");

            if (SpuConfig.change_xa_speed)
                sprintf(options.value[1], "Enabled");
            else
                sprintf(options.value[1], "Disabled");

            if (SpuConfig.irq_wait)
                sprintf(options.value[2], "Enabled");
            else
                sprintf(options.value[2], "Disabled");


            optionBrowser.TriggerUpdate();
        }

        if (backBtn.GetState() == STATE_CLICKED) {
            if (pcsxr_running == 0)
                menu = MENU_MAIN;
            else
                menu = MENU_IN_GAME;
        }

    }
    HaltGui();
    mainWindow->Remove(&optionBrowser);
    mainWindow->Remove(&w);
    mainWindow->Remove(&titleTxt);
    return menu;
}

static int MenuConfigGPU() {
    int menu = MENU_NONE;
    int ret;
    int i = 0;
    bool firstRun = true;

    TR;


    //settings_load();

    OptionList options;
    sprintf(options.name[i++], "Fps Limit");
    sprintf(options.name[i++], "Hi Res texture");
    sprintf(options.name[i++], "GTE Accuracy");
    options.length = i;

    for (i = 0; i < options.length; i++)
        options.value[i][0] = 0;

    GuiText titleTxt(PCSXR_APP_NAME" - GPU Options", 28, ColorGrey);
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    titleTxt.SetPosition(50, 50);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(xenon_button_png);
    GuiImageData btnOutlineOver(xenon_button_over_png);

    GuiTrigger trigA;
    //	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigA.SetSimpleTrigger(-1, 0, PAD_BUTTON_A);

    GuiText backBtnTxt("Go Back", 22, ColorGrey2);
    GuiImage backBtnImg(&btnOutline);
    GuiImage backBtnImgOver(&btnOutlineOver);
    GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    backBtn.SetPosition(100, -35);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetImage(&backBtnImg);
    backBtn.SetImageOver(&backBtnImgOver);
    backBtn.SetSoundOver(&btnSoundOver);
    backBtn.SetTrigger(&trigA);
    backBtn.SetEffectGrow();

    GuiOptionBrowser optionBrowser(1080, 496, &options);
    optionBrowser.SetPosition(0, 108);
    optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    optionBrowser.SetCol2Position(500);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&backBtn);
    mainWindow->Append(&optionBrowser);
    mainWindow->Append(&w);
    mainWindow->Append(&titleTxt);
    ResumeGui();

    while (menu == MENU_NONE) {
        TH_UGUI();
        usleep(THREAD_SLEEP);

        ret = optionBrowser.GetClickedOption();

        switch (ret) {
            case 0:
                HwGpuConfig.fps_limit++;
                if (HwGpuConfig.fps_limit > 1)
                    HwGpuConfig.fps_limit = 0;
                break;
            case 1:
                HwGpuConfig.hires_texture++;
                if (HwGpuConfig.hires_texture > 3)
                    HwGpuConfig.hires_texture = 0;
                break;
            case 2:
                HwGpuConfig.gte_accuracy++;
                if (HwGpuConfig.gte_accuracy > 1)
                    HwGpuConfig.gte_accuracy = 0;
                break;
        }

        if (ret >= 0 || firstRun) {
            firstRun = false;

            if (HwGpuConfig.fps_limit)
                sprintf(options.value[0], "On");
            else
                sprintf(options.value[0], "Off");

            if (HwGpuConfig.gte_accuracy)
                sprintf(options.value[2], "On");
            else
                sprintf(options.value[2], "Off");

            switch (HwGpuConfig.hires_texture) {
                case 0:
                    sprintf(options.value[1], "None");
                    break;
                case 1:
                    sprintf(options.value[1], "2XSaI");
                    break;
                case 2:
                    sprintf(options.value[1], "Stretched");
                    break;
            }

            optionBrowser.TriggerUpdate();
        }

        if (backBtn.GetState() == STATE_CLICKED) {
            if (pcsxr_running == 0)
                menu = MENU_MAIN;
            else
                menu = MENU_IN_GAME;
        }

    }
    HaltGui();
    mainWindow->Remove(&optionBrowser);
    mainWindow->Remove(&w);
    mainWindow->Remove(&titleTxt);
    return menu;
}

/****************************************************************************
 * FindGameSaveNum
 *
 * Determines the save file number of the given file name
 * Returns -1 if none is found
 ***************************************************************************/
static int FindGameSaveNum(char * savefile, int device) {
    printf("savefile => %s\r\n", savefile);
    int n = -1;
    int romlen = strlen(ROMFilename);
    int savelen = strlen(savefile);

    int diff = savelen - romlen;

    if (strncmp(savefile, ROMFilename, romlen) != 0)
        return -1;

    if (savefile[romlen] == ' ') {
        if (diff == 5 && strncmp(&savefile[romlen + 1], "Auto", 4) == 0)
            n = 0; // found Auto save
        else if (diff == 2 || diff == 3)
            n = atoi(&savefile[romlen + 1]);
    }

    if (n >= 0 && n < MAX_SAVES)
        return n;
    else
        return -1;
}

static int MenuSaveStates(int action) {
    int menu = MENU_NONE;
    int ret;
    int i, n, type, len, len2;
    int j = 0;
    SaveList saves;
    char filepath[1024];
    char scrfile[1024];
    char tmp[MAXJOLIET + 1];
    struct stat filestat;
    struct tm * timeinfo;

    GuiText titleTxt(NULL, 26, (GXColor) {
        255, 255, 255, 255
    });
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    titleTxt.SetPosition(50, 50);

    if (action == 0)
        titleTxt.SetText(PCSXR_APP_NAME" - Load Game");
    else
        titleTxt.SetText(PCSXR_APP_NAME" - Save Game");

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(xenon_button_png);
    GuiImageData btnOutlineOver(xenon_button_over_png);
    GuiImageData btnCloseOutline(xenon_button_png);
    GuiImageData btnCloseOutlineOver(xenon_button_over_png);

    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, 0, PAD_BUTTON_A);

    GuiText backBtnTxt("Go Back", 22, ColorGrey2);
    GuiImage backBtnImg(&btnOutline);
    GuiImage backBtnImgOver(&btnOutlineOver);
    GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    backBtn.SetPosition(100, -35);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetImage(&backBtnImg);
    backBtn.SetImageOver(&backBtnImgOver);
    backBtn.SetSoundOver(&btnSoundOver);
    backBtn.SetSoundClick(&btnSoundClick);
    backBtn.SetTrigger(&trigA);
    //    backBtn.SetTrigger(trig2);
    backBtn.SetEffectGrow();

    GuiText closeBtnTxt("Close", 20, (GXColor) {
        0, 0, 0, 255
    });
    GuiImage closeBtnImg(&btnCloseOutline);
    GuiImage closeBtnImgOver(&btnCloseOutlineOver);
    GuiButton closeBtn(btnCloseOutline.GetWidth(), btnCloseOutline.GetHeight());
    closeBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    closeBtn.SetPosition(-50, 35);
    closeBtn.SetLabel(&closeBtnTxt);
    closeBtn.SetImage(&closeBtnImg);
    closeBtn.SetImageOver(&closeBtnImgOver);
    closeBtn.SetSoundOver(&btnSoundOver);
    closeBtn.SetSoundClick(&btnSoundClick);
    closeBtn.SetTrigger(&trigA);
    //    closeBtn.SetTrigger(trig2);
    closeBtn.SetTrigger(&trigHome);
    closeBtn.SetEffectGrow();

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    //w.Append(&backBtn);
    w.Append(&closeBtn);
    mainWindow->Append(&w);
    mainWindow->Append(&titleTxt);
    ResumeGui();

    memset(&saves, 0, sizeof (saves));

    //len = strlen(ROMFilename);

    sprintf(foldername, "/states/");

    printf("foldername : %s\r\n", foldername);
    BrowseDevice("/states/", "uda:/");

    printf("browser.dir : %s\r\n", browser.dir);
    printf("rootdir : %s\r\n", rootdir);

    for (i = 0; i < browser.numEntries; i++) {
        len2 = strlen(browserList[i].filename);

        if (len2 < 6 || len2 - len < 5)
            continue;

        if (strncmp(&browserList[i].filename[len2 - 4], ".gpz", 4) == 0) {
            type = FILE_SNAPSHOT;
        } else {
            continue;
        }

        printf("found : %s\r\n", browserList[i].filename);

        strcpy(tmp, browserList[i].filename);
        tmp[len2 - 4] = 0;
        n = FindGameSaveNum(tmp, 0);

        if (n >= 0) {
            saves.type[j] = type;
            saves.files[saves.type[j]][n] = 1;
            strcpy(saves.filename[j], browserList[i].filename);

            if (saves.type[j] == FILE_SNAPSHOT) {
                //                sprintf(scrfile, "%s%s/%s.png", pathPrefix[GCSettings.SaveMethod], GCSettings.SaveFolder, tmp);
                //
                //                memset(savebuffer, 0, SAVEBUFFERSIZE);
                //                if (LoadFile(scrfile, SILENT))
                //                    saves.previewImg[j] = new GuiImageData(savebuffer, 64, 48);
            }
            snprintf(filepath, 1024, "uda:/%s/%s", foldername, saves.filename[j]);
            printf("filepath : %s\r\n", filepath);
            if (stat(filepath, &filestat) == 0) {
                timeinfo = localtime(&filestat.st_mtime);
                sprintf(saves.date[j], "Save %d", j);
                //strftime(saves.date[j], 20, "%a %b %d", timeinfo);
                //strftime(saves.time[j], 10, "%I:%M %p", timeinfo);
            }
            j++;
        }
    }

    //    FreeSaveBuffer();
    saves.length = j;

    if (saves.length == 0 && action == 0) {
        InfoPrompt("No game saves found.");
        menu = MENU_IN_GAME;
    }

    GuiSaveBrowser saveBrowser(1080, 496, &saves, action);
    saveBrowser.SetPosition(0, 108);
    saveBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

    HaltGui();
    mainWindow->Append(&saveBrowser);
    mainWindow->ChangeFocus(&saveBrowser);
    ResumeGui();

    while (menu == MENU_NONE) {
        TH_UGUI();
        usleep(THREAD_SLEEP);

        ret = saveBrowser.GetClickedSave();

        // load or save game
        if (ret > -3) {
            TR;
            int result = 0;

            if (action == 0) // load
            {
                TR;
                MakeFilePath(filepath, FILE_SNAPSHOT, saves.filename[ret]);
                LoadState(filepath);
                menu = MENU_EMULATION;
            } else // save
            {
                TR;
                if (ret == -3) // overwrite SRAM/Snapshot
                {
                    TR;
                    MakeFilePath(filepath, FILE_SNAPSHOT, saves.filename[ret]);
                    SaveState(filepath);
                    menu = MENU_SAVE;
                } else // new Snapshot
                {
                    for (i = 1; i < 100; i++)
                        if (saves.files[FILE_SNAPSHOT][i] == 0)
                            break;
                    TR;
                    printf("%d\r\n", i);
                    if (i < 100) {
                        TR;

                        //MakeFilePath(filepath, FILE_SNAPSHOT, ROMFilename, i);
                        MakeFilePath(filepath, FILE_SNAPSHOT, ROMFilename, i);

                        printf("filepath :%s\r\n", filepath);
                        printf("ROMFilename :%s\r\n", ROMFilename);

                        SaveState(filepath);
                        menu = MENU_SAVE;
                    }
                }
            }
        }

        if (backBtn.GetState() == STATE_CLICKED) {
            menu = MENU_IN_GAME;
        } else if (closeBtn.GetState() == STATE_CLICKED) {
            menu = MENU_IN_GAME;

            //            exitSound->Play();
            //            bgTopImg->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
            closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
            titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
            backBtn.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
            //            bgBottomImg->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
            //            btnLogo->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);

            w.SetEffect(EFFECT_FADE, -15);

            usleep(350000); // wait for effects to finish
        }
    }

    HaltGui();

    for (i = 0; i < saves.length; i++)
        if (saves.previewImg[i])
            delete saves.previewImg[i];

    mainWindow->Remove(&saveBrowser);
    mainWindow->Remove(&w);
    mainWindow->Remove(&titleTxt);
    ResetBrowser();
    return menu;
}

static int MenuCheats() {
    int menu = MENU_NONE;
    return menu;
}

static int MenuInGame() {
    int menu = MENU_NONE;

    GuiText titleTxt(PCSXR_APP_NAME" In game menu", 28, ColorGrey);
    titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    titleTxt.SetPosition(50, 50);

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    GuiImageData btnOutline(xenon_button_png);
    GuiImageData btnOutlineOver(xenon_button_over_png);
    GuiImageData btnLargeOutline(xenon_button_large_png);
    GuiImageData btnLargeOutlineOver(xenon_button_large_over_png);

    GuiTrigger trigA;
    trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    //    trigA.SetSimpleTrigger(-1, 0, PAD_BUTTON_A);
    GuiTrigger trigHome;
    trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, PAD_BUTTON_LOGO);
    //    trigHome.SetButtonOnlyTrigger(-1, 0, PAD_BUTTON_LOGO);

    GuiText fileBtnTxt("New game", 18, ColorGrey2);
    fileBtnTxt.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    GuiImage fileBtnImg(&btnLargeOutline);
    GuiImage fileBtnImgOver(&btnLargeOutlineOver);
    GuiButton fileBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    fileBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    fileBtn.SetPosition(50, 120);
    fileBtn.SetLabel(&fileBtnTxt);
    fileBtn.SetImage(&fileBtnImg);
    fileBtn.SetImageOver(&fileBtnImgOver);
    fileBtn.SetSoundOver(&btnSoundOver);
    fileBtn.SetTrigger(&trigA);
    fileBtn.SetEffectGrow();

    GuiText configspuBtnTxt("SPU Config", 18, ColorGrey2);
    configspuBtnTxt.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    GuiImage configspuBtnImg(&btnLargeOutline);
    GuiImage configspuBtnImgOver(&btnLargeOutlineOver);
    GuiButton configspuBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    configspuBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    configspuBtn.SetPosition(250, 120);
    configspuBtn.SetLabel(&configspuBtnTxt);
    configspuBtn.SetImage(&configspuBtnImg);
    configspuBtn.SetImageOver(&configspuBtnImgOver);
    configspuBtn.SetSoundOver(&btnSoundOver);
    configspuBtn.SetTrigger(&trigA);
    configspuBtn.SetEffectGrow();

    GuiText configgpuBtnTxt("GPU Config", 18, ColorGrey2);
    configgpuBtnTxt.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    GuiImage configgpuBtnImg(&btnLargeOutline);
    GuiImage configgpuBtnImgOver(&btnLargeOutlineOver);
    GuiButton configgpuBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    configgpuBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    configgpuBtn.SetPosition(450, 120);
    configgpuBtn.SetLabel(&configgpuBtnTxt);
    configgpuBtn.SetImage(&configgpuBtnImg);
    configgpuBtn.SetImageOver(&configgpuBtnImgOver);
    configgpuBtn.SetSoundOver(&btnSoundOver);
    configgpuBtn.SetTrigger(&trigA);
    configgpuBtn.SetEffectGrow();

    GuiText loadingBtnTxt1("Load", 18, ColorGrey2);
    loadingBtnTxt1.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    GuiImage loadingBtnImg(&btnLargeOutline);
    GuiImage loadingBtnImgOver(&btnLargeOutlineOver);
    GuiButton loadingBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    loadingBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    loadingBtn.SetPosition(650, 120);
    loadingBtn.SetLabel(&loadingBtnTxt1);
    loadingBtn.SetImage(&loadingBtnImg);
    loadingBtn.SetImageOver(&loadingBtnImgOver);
    loadingBtn.SetSoundOver(&btnSoundOver);
    loadingBtn.SetTrigger(&trigA);
    loadingBtn.SetEffectGrow();

    GuiText savingBtnTxt1("Save", 18, ColorGrey2);
    savingBtnTxt1.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    GuiImage savingBtnImg(&btnLargeOutline);
    GuiImage savingBtnImgOver(&btnLargeOutlineOver);
    GuiButton savingBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    savingBtn.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    savingBtn.SetPosition(850, 120);
    savingBtn.SetLabel(&savingBtnTxt1);
    savingBtn.SetImage(&savingBtnImg);
    savingBtn.SetImageOver(&savingBtnImgOver);
    savingBtn.SetSoundOver(&btnSoundOver);
    savingBtn.SetTrigger(&trigA);
    savingBtn.SetEffectGrow();


    //
    //    GuiText cheatsBtnTxt("Cheats", 18, ColorGrey2);
    //    cheatsBtnTxt.SetWrap(true, btnLargeOutline.GetWidth() - 30);
    //    GuiImage cheatsBtnImg(&btnLargeOutline);
    //    GuiImage cheatsBtnImgOver(&btnLargeOutlineOver);
    //    GuiButton cheatsBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
    //    cheatsBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    //    cheatsBtn.SetPosition(0, 250);
    //    cheatsBtn.SetLabel(&cheatsBtnTxt);
    //    cheatsBtn.SetImage(&cheatsBtnImg);
    //    cheatsBtn.SetImageOver(&cheatsBtnImgOver);
    //    cheatsBtn.SetSoundOver(&btnSoundOver);
    //    cheatsBtn.SetTrigger(&trigA);
    //    cheatsBtn.SetEffectGrow();

    GuiText backBtnTxt("Return to game", 22, ColorGrey2);
    GuiImage backBtnImg(&btnOutline);
    GuiImage backBtnImgOver(&btnOutlineOver);
    GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
    backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    backBtn.SetPosition(100, -35);
    backBtn.SetLabel(&backBtnTxt);
    backBtn.SetImage(&backBtnImg);
    backBtn.SetImageOver(&backBtnImgOver);
    backBtn.SetSoundOver(&btnSoundOver);
    backBtn.SetTrigger(&trigA);
    backBtn.SetEffectGrow();

    //    GuiText about_btnTxt("About", 18, ColorGrey2);
    //    GuiImage about_btnImg(&btnOutline);
    //    GuiImage about_btnImgOver(&btnOutlineOver);
    //    GuiButton about_btn(btnOutline.GetWidth(), btnOutline.GetHeight());
    //    about_btn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
    //    about_btn.SetPosition(450, -35);
    //    about_btn.SetLabel(&about_btnTxt);
    //    about_btn.SetImage(&about_btnImg);
    //    about_btn.SetImageOver(&about_btnImgOver);
    //    about_btn.SetSoundOver(&btnSoundOver);
    //    about_btn.SetTrigger(&trigA);
    //    about_btn.SetTrigger(&trigHome);
    //    about_btn.SetEffectGrow();


    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&titleTxt);
    w.Append(&savingBtn);
    w.Append(&loadingBtn);
    //w.Append(&cheatsBtn);
    w.Append(&backBtn);
    w.Append(&fileBtn);
    w.Append(&configgpuBtn);
    w.Append(&configspuBtn);

    mainWindow->Append(&w);

    ResumeGui();

    while (menu == MENU_NONE) {
        TH_UGUI();
        usleep(THREAD_SLEEP);

        if (savingBtn.GetState() == STATE_CLICKED) {
            menu = MENU_SAVE;
        } else if (loadingBtn.GetState() == STATE_CLICKED) {
            menu = MENU_LOAD;
        } else if (configgpuBtn.GetState() == STATE_CLICKED) {
            menu = MENU_GPU;
        } else if (configspuBtn.GetState() == STATE_CLICKED) {
            menu = MENU_SPU;
        }            /*
        else if (cheatsBtn.GetState() == STATE_CLICKED) {
            menu = MENU_CHEATS;
        }
         */
        else if (fileBtn.GetState() == STATE_CLICKED) {
            if (WindowPrompt("Load", "Load a new game", "Ok", "Cancel")) {
                menu = MENU_BROWSE_DEVICE;
            }
        } else if (backBtn.GetState() == STATE_CLICKED) {
            menu = MENU_EMULATION;

        } /*
        else if (about_btn.GetState() == STATE_CLICKED) {
            about_btn.ResetState();

            InfoPrompt(
                    PCSXR_NAME
                    );

        }*/


    }

    HaltGui();
    mainWindow->Remove(&w);
    return menu;
}

static int MenuEmulation() {
    pcsxr_running = 1;
    // fix input repitition
    usb_do_poll();
    psxCpu->Execute();
    return MENU_IN_GAME;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
void MainMenu(int menu) {


    TR;
    int currentMenu = menu;

    GuiImageData * background = new GuiImageData(xenon_bg_png);

    mainWindow = new GuiWindow(screenwidth, screenheight);

    bgImg = new GuiImage(background);

    HaltGui();
    mainWindow->Append(bgImg);
    ResumeGui();

    //    bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG);
    //    bgMusic->SetVolume(50);
    //    bgMusic->Play(); // startup music

    TR;
    while (currentMenu != MENU_EXIT) {
        switch (currentMenu) {
            case MENU_MAIN:
                currentMenu = MenuMain();
                break;
            case MENU_OPTIONS:
                currentMenu = MenuConfig();
                break;
            case MENU_SPU:
                currentMenu = MenuConfigSPU();
                break;
            case MENU_GPU:
                currentMenu = MenuConfigGPU();
                break;
            case MENU_SAVE:
                currentMenu = MenuSaveStates(1);
                break;
            case MENU_LOAD:
                currentMenu = MenuSaveStates(0);
                break;
            case MENU_CHEATS:
                currentMenu = MenuCheats();
                break;
            case MENU_BROWSE_DEVICE:
                currentMenu = MenuBrowseDevice();
                break;
            case MENU_EMULATION:
                currentMenu = MenuEmulation();
                break;
            case MENU_IN_GAME:
                currentMenu = MenuInGame();
                break;
            default:
                currentMenu = menu;
                break;
        }
    }
    ResumeGui();

    while (1) {
        udelay(THREAD_SLEEP);
    }

    HaltGui();

    //    bgMusic->Stop();
    //    delete bgMusic;
    delete bgImg;
    delete mainWindow;

    delete pointer[0];
    delete pointer[1];
    delete pointer[2];
    delete pointer[3];

    mainWindow = NULL;
}

int main() {
    xenos_init(VIDEO_MODE_HDMI_720P);
    //xenos_init(VIDEO_MODE_AUTO);
    xenon_sound_init();
    console_init();

//    ntfs_vfs_init();
//    ext2fs_init();

    xenon_make_it_faster(XENON_SPEED_FULL);

    usb_init();
    usb_do_poll();

    InitFreeType((u8*) font_ttf, font_ttf_size); // Initialize font system

    SetupPads();
    InitVideo();

    console_close();

    // run gui
    InitGUIThreads();

    SpuConfig.change_xa_speed = 1;
    SpuConfig.enable_xa = 1;
    SpuConfig.irq_wait = 1;

    HwGpuConfig.fps_limit = 1;
    HwGpuConfig.gte_accuracy = 1;
    HwGpuConfig.hires_texture = 1;


    memset(&Config, 0, sizeof (PcsxConfig));

    MainMenu(MENU_MAIN);

    return 0;
}

#endif