/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_filebrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "../g_filebrowser.h"
#include "../w_input.h"
#include "../gui_debug.h"

//#define FILE_PAGESIZE 20

/**
 * Constructor for the GuiFileBrowser class.
 */
GuiFileBrowser::GuiFileBrowser(int w, int h) {
    width = w;
    height = h;
    numEntries = 0;
    selectedItem = 0;
    selectable = true;
    listChanged = true; // trigger an initial list update
    focus = 0; // allow focus

    trigA = new GuiTrigger;
    //	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigA->SetSimpleTrigger(-1, 0, PAD_BUTTON_A);
    trig2 = new GuiTrigger;
    //	trig2->SetSimpleTrigger(-1, WPAD_BUTTON_2, 0);
    trig2->SetSimpleTrigger(-1, 0, 0);

    trigHeldA = new GuiTrigger;
    //	trigHeldA->SetHeldTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
    trigHeldA->SetHeldTrigger(-1, 0, PAD_BUTTON_A);

    btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
    btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

    bgFileSelection = new GuiImageData(xenon_filebrowser_png);
    bgFileSelectionImg = new GuiImage(bgFileSelection);
    bgFileSelectionImg->SetParent(this);
    bgFileSelectionImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

    bgFileSelectionEntry = new GuiImageData(xenon_bg_file_selection_entry_png);
    fileFolder = new GuiImageData(folder_png);

    for (int i = 0; i < FILE_PAGESIZE; ++i) {

        fileListText[i] = new GuiText(NULL, 20, (GXColor) {
            0xff, 0xff, 0xff, 0xff
        });
        fileListText[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
        fileListText[i]->SetPosition(5, 0);
        fileListText[i]->SetMaxWidth(1024);

        fileListBg[i] = new GuiImage(bgFileSelectionEntry);
        fileListFolder[i] = new GuiImage(fileFolder);

        fileList[i] = new GuiButton(1024, 30);
        fileList[i]->SetParent(this);
        fileList[i]->SetLabel(fileListText[i]);
        fileList[i]->SetImageOver(fileListBg[i]);
        fileList[i]->SetPosition(2, 30 * i + 3);
        fileList[i]->SetTrigger(trigA);
        fileList[i]->SetTrigger(trig2);
        fileList[i]->SetSoundClick(btnSoundClick);
    }
}

/**
 * Destructor for the GuiFileBrowser class.
 */
GuiFileBrowser::~GuiFileBrowser() {
    delete bgFileSelectionImg;

    delete bgFileSelection;
    delete bgFileSelectionEntry;
    delete fileFolder;

    delete btnSoundOver;
    delete btnSoundClick;
    delete trigHeldA;
    delete trigA;
    delete trig2;

    for (int i = 0; i < FILE_PAGESIZE; i++) {
        delete fileListText[i];
        delete fileList[i];
        delete fileListBg[i];
        delete fileListFolder[i];
    }
}

void GuiFileBrowser::SetFocus(int f) {
    focus = f;

    for (int i = 0; i < FILE_PAGESIZE; i++)
        fileList[i]->ResetState();

    if (f == 1)
        fileList[selectedItem]->SetState(STATE_SELECTED);
}

void GuiFileBrowser::ResetState() {
    state = STATE_DEFAULT;
    stateChan = -1;
    selectedItem = 0;

    for (int i = 0; i < FILE_PAGESIZE; i++) {
        fileList[i]->ResetState();
    }
}

void GuiFileBrowser::TriggerUpdate() {
    listChanged = true;
}

/**
 * Draw the button on screen
 */
void GuiFileBrowser::Draw() {
    if (!this->IsVisible())
        return;

    bgFileSelectionImg->Draw();

    for (u32 i = 0; i < FILE_PAGESIZE; ++i) {
        fileList[i]->Draw();
    }

    this->UpdateEffects();
}

void GuiFileBrowser::DrawTooltip() {
}

void GuiFileBrowser::Update(GuiTrigger * t) {
    if (state == STATE_DISABLED || !t)
        return;

    int position = 0;
    int positionWiimote = 0;


    // pad/joystick navigation
    if (!focus) {
        goto endNavigation; // skip navigation
        listChanged = false;
    }

    if (t->Right()) {
        if (browser.pageIndex < browser.numEntries && browser.numEntries > FILE_PAGESIZE) {
            browser.pageIndex += FILE_PAGESIZE;
            if (browser.pageIndex + FILE_PAGESIZE >= browser.numEntries)
                browser.pageIndex = browser.numEntries - FILE_PAGESIZE;
            listChanged = true;
        }
    } else if (t->Left()) {
        if (browser.pageIndex > 0) {
            browser.pageIndex -= FILE_PAGESIZE;
            if (browser.pageIndex < 0)
                browser.pageIndex = 0;
            listChanged = true;
        }
    } else if (t->Down()) {
        if (browser.pageIndex + selectedItem + 1 < browser.numEntries) {
            if (selectedItem == FILE_PAGESIZE - 1) {
                // move list down by 1
                ++browser.pageIndex;
                listChanged = true;
            } else if (fileList[selectedItem + 1]->IsVisible()) {
                fileList[selectedItem]->ResetState();
                fileList[++selectedItem]->SetState(STATE_SELECTED, t->chan);
            }
        }
    } else if (t->Up()) {
        if (selectedItem == 0 && browser.pageIndex + selectedItem > 0) {
            // move list up by 1
            --browser.pageIndex;
            listChanged = true;
        } else if (selectedItem > 0) {
            fileList[selectedItem]->ResetState();
            fileList[--selectedItem]->SetState(STATE_SELECTED, t->chan);
        }
    }

endNavigation:

    for (int i = 0; i < FILE_PAGESIZE; ++i) {
        if (listChanged || numEntries != browser.numEntries) {
            if (browser.pageIndex + i < browser.numEntries) {
                if (fileList[i]->GetState() == STATE_DISABLED)
                    fileList[i]->SetState(STATE_DEFAULT);

                fileList[i]->SetVisible(true);

                fileListText[i]->SetText(browserList[browser.pageIndex + i].displayname);

                if (browserList[browser.pageIndex + i].isdir) // directory
                {
                    fileList[i]->SetIcon(fileListFolder[i]);
                    fileListText[i]->SetPosition(30, 0);
                } else {
                    fileList[i]->SetIcon(NULL);
                    fileListText[i]->SetPosition(10, 0);
                }
            } else {
                fileList[i]->SetVisible(false);
                fileList[i]->SetState(STATE_DISABLED);
            }
        }

        if (i != selectedItem && fileList[i]->GetState() == STATE_SELECTED)
            fileList[i]->ResetState();
        else if (focus && i == selectedItem && fileList[i]->GetState() == STATE_DEFAULT)
            fileList[selectedItem]->SetState(STATE_SELECTED, t->chan);

        int currChan = t->chan;

        if (t->wpad->ir.valid && !fileList[i]->IsInside(t->wpad->ir.x, t->wpad->ir.y))
            t->chan = -1;

        fileList[i]->Update(t);
        t->chan = currChan;

        if (fileList[i]->GetState() == STATE_SELECTED) {
            selectedItem = i;
            browser.selIndex = browser.pageIndex + i;
        }

        if (selectedItem == i)
            fileListText[i]->SetScroll(SCROLL_HORIZONTAL);
        else
            fileListText[i]->SetScroll(SCROLL_NONE);
    }

    listChanged = false;
    numEntries = browser.numEntries;

    if (updateCB)
        updateCB(this);
}
