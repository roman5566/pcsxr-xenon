#if 0
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
#define PCSXR_VERSION   "0.6"
#define PCSXR_NAME      "PCSXR Xenon"
#include "psxcommon.h"
#include "config.h"
#include "r3000a.h"
#include "debug.h"
#include "sio.h"
#include "misc.h"
#include "gamecube_plugins.h"


#include "settings.h"

enum {
    MENU_EXIT = -1,
    MENU_NONE,
    MENU_MAIN,
    MENU_OPTIONS,
    MENU_CHEATS,
    MENU_SAVE,
    MENU_SETTINGS,
    MENU_SETTINGS_FILE,
    MENU_IN_GAME,
    MENU_BROWSE_DEVICE,
    MENU_GAME_SAVE,
    MENU_GAME_LOAD,
    MENU_EMULATION
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

    sprintf(title, "%s %s  - Load Game", PCSXR_NAME, PCSXR_VERSION);

    GuiText titleTxt(title, 28, ColorGrey);
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

    GuiText backBtnTxt("Go Back", 18, ColorGrey2);
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

struct controller_data_s ctrl;
struct controller_data_s old_ctrl;

void systemPoll() {
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
        if (WindowPrompt("Exit", "exit to menu", "Ok", "Cancel")) {
            pcsxr_running = 0;
            psxCpu->Shutdown();
        }
        pcsxr_exit_asked=0;
    }
}

void gui_vsync(){
    TH_UGUI();
}

static int MenuMain(){
    int menu = MENU_NONE;
    settings_load();
    

    GuiText titleTxt(PCSXR_NAME , 28, ColorGrey);
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

    HaltGui();
    GuiWindow w(screenwidth, screenheight);
    w.Append(&titleTxt);
    w.Append(&fileBtn);
    w.Append(&optionBtn);
    w.Append(&shutdown_btn);
    //    w.Append(&savingBtn);
    // w.Append(&cheatsBtn); // unused
    w.Append(&exitBtn);

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
        } else if (cheatsBtn.GetState() == STATE_CLICKED) {
            menu = MENU_CHEATS;
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
                "Genesis plus Xenon ..."
            );

        }
    }

    HaltGui();
    mainWindow->Remove(&w);
    return menu;
}

static int MenuConfig(){
    int menu = MENU_NONE;
    int ret;
    int i = 0;
    bool firstRun = true;

    TR;
    
    //settings_load();
    
    OptionList options;
    for(i=0;i<c_emu_settings.len();i++){
        //options[i].name
        TR;
        settings_entry * s = c_emu_settings.getEntry(i);
        s->getName(options.name[i]);
        TR;
        strcpy(options.value[i], s->getSelectedValueName());
        TR;
    }
    
    TR;
    
    options.length = i;

    GuiText titleTxt("Genesis Plus Xenon - Options", 28, ColorGrey);
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
    optionBrowser.SetCol2Position(185);

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

        if (ret >= 0) {
            firstRun = false;

            int v  = c_emu_settings.getEntry(ret)->getValue();
            c_emu_settings.getEntry(ret)->setValue(v++);
            
            // update strings
            sprintf(options.value[ret], c_emu_settings.getEntry(ret)->getSelectedValueName());

            // Update label
            optionBrowser.TriggerUpdate();
        }

        if (backBtn.GetState() == STATE_CLICKED) {
            
        }

    }
    HaltGui();
    mainWindow->Remove(&optionBrowser);
    mainWindow->Remove(&w);
    mainWindow->Remove(&titleTxt);
    return menu;
}

static int MenuConfigSPU(){
    
}

static int MenuConfigGPU(){
    
}

static int MenuSaveStates(){
    
}

static int MenuEmulation() {
    char cdfile[2048];
    int menu = MENU_BROWSE_DEVICE;
    sprintf(cdfile, "%s/%s/%s", rootdir, browser.dir, browserList[browser.selIndex].filename);

    memset(&Config, 0, sizeof (PcsxConfig));

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

    Config.PsxOut = 0; // Enable Console Output 
    Config.SpuIrq = 0; // Spu Irq Always Enabled
    //Config.HLE = 0; 
    Config.Xa = 0; // Disable Xa Decoding
    Config.Cdda = 0; // Disable Cd audio
    Config.PsxAuto = 1; // autodetect system
    //Config.PsxType = PSX_TYPE_NTSC;
    Config.Cpu = CPU_DYNAREC;
    
    pcsxr_running = 0;

    SetIso(cdfile);
    if (LoadPlugins() == 0) {
        if (OpenPlugins() == 0) {
            if (SysInit() == -1) {
                ErrorPrompt("SysInit() Error!\n");
                return menu;
            }

            SysReset();
            // Check for hle ...
            if (Config.HLE == 1) {
                ErrorPrompt("Can't continue ... bios not found ...");
                return menu;
            }

            int ret = CheckCdrom();
            if (CheckCdrom() != 0) {
                ErrorPrompt("Can't continue ... invalide cd-image detected ...");
                return menu;
            }
            ret = LoadCdrom();
            if (ret != 0) {
                ErrorPrompt("Can't continue ... no executable found ...");
                return menu;
            }
            pcsxr_running = 1;
            psxCpu->Execute();
        }
    }

    return MENU_BROWSE_DEVICE;
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
            case MENU_BROWSE_DEVICE:
                currentMenu = MenuBrowseDevice();
                break;
            case MENU_EMULATION:
                currentMenu = MenuEmulation();
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
    xenon_sound_init();
    console_init();

    xenon_make_it_faster(XENON_SPEED_FULL);

    usb_init();
    usb_do_poll();

    InitFreeType((u8*) font_ttf, font_ttf_size); // Initialize font system

    SetupPads();
    InitVideo();

    console_close();

    // run gui
    InitGUIThreads();

    MainMenu(MENU_MAIN);

    return 0;
}
#endif