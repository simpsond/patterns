#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/asl.h>
#include <proto/muimaster.h>
#include <libraries/mui.h>
#include <BetterString_mcc.h>
#include <TextEditor_mcc.h>
#include <stdio.h>
#include <string.h>

#include "gui_helpers.h"
#include "utils.h"

enum FileDialogType {
  DIALOG_FILE = 0,
  DIALOG_DIRECTORY
};

struct DosBase* DosBase;
struct Library* IntuitionBase;
struct MUIMasterBase* MUIMasterBase;
struct ASLBase* AslBase;

APTR app;
APTR window;

// Project Menu items
APTR quit_menu;
APTR import_file_menu;
APTR import_directory_menu;

// Edit Menu items
APTR copy_menu;
APTR copy_matches_menu;
APTR paste_menu;

// Main window gadgets
APTR glob_button;
APTR glob_text;
APTR search_text_editor;


void OnGlobItClick(void);
void OnQuitMenu(void);
void OnImportFileMenu(void);
void OnImportDirectoryMenu(void);
void OnCopyMatchesMenu(void);

BOOL DialogGetFileSystemPath(enum FileDialogType type, STRPTR fname);

void fail(APTR app,char *str);
void ProcessGlobLine(STRPTR in_string, STRPTR out_string, APTR extra);
void CreateApplicationObject();
void ConfigureAndBind();
void InitLibs();
void FreeLibs();

int main() {
  printf("Starting Patterns!\n");

  InitLibs();

  CreateApplicationObject();
  if (!app) {
    fail(app, "Failed to create Application.");
  }
  ConfigureAndBind();

  set(window,MUIA_Window_Open,TRUE);

  ULONG sigs = 0;

    while (DoMethod(app,MUIM_Application_NewInput,&sigs) != MUIV_Application_ReturnID_Quit) {
      if (sigs) {
        sigs = Wait(sigs | SIGBREAKF_CTRL_C);
        if (sigs & SIGBREAKF_CTRL_C) break;
      }
    }

  set(window,MUIA_Window_Open,FALSE);

  fail(app, NULL);
  return 0;
}

void InitLibs() {
  DosBase = (struct DosBase*)OpenLibrary("dos.library", 0);
  IntuitionBase = OpenLibrary("intuition.library", 0);
  AslBase = OpenLibrary("asl.library", 0);
  if (!(MUIMasterBase = OpenLibrary(MUIMASTER_NAME,MUIMASTER_VMIN))) {
    fail(NULL, "Failed to open " MUIMASTER_NAME ".");
  }
}

void FreeLibs() {
  if(MUIMasterBase) {
    CloseLibrary((struct Library *) MUIMasterBase);
  }
  if(DosBase) {
    CloseLibrary((struct Library *) DosBase);
  }
  if(AslBase) {
    CloseLibrary((struct Library *) AslBase);
  }
  if(IntuitionBase) {
    CloseLibrary((struct Library *) IntuitionBase);
  }
}

void fail(APTR app, char *str) {
  if (app) {
    MUI_DisposeObject(app);
  }

  FreeLibs();

  if (str) {
    puts(str);
    exit(20);
  }
  exit(0);
}

void ProcessGlobLine(STRPTR in_string, STRPTR out_string, APTR pattern_string) {
  if(CheckStringPattern((STRPTR)pattern_string, in_string) == 1) {
    HighlightAndAppendControl(in_string, search_text_editor);
  } else {
    AppendControl(in_string, search_text_editor);
  }
}

void CreateApplicationObject() {
  app =  ApplicationObject,
    MUIA_Application_Title      , "Patterns",
    MUIA_Application_Version    , "$VER: Patterns 1.0 (27.02.09)",
    MUIA_Application_Copyright  , "(c) Dustin Simpson",
    MUIA_Application_Author     , "Dustin Simpson",
    MUIA_Application_Description, "Pattern matching helper",
    MUIA_Application_Base       , "PROGRAM",
    MUIA_Application_Menustrip, MenustripObject,
      MUIA_Family_Child, MenuObject,
        MUIA_Menu_Title, "Project",
          MUIA_Family_Child,
            import_file_menu = MenuitemObject, MUIA_Menuitem_Title, "Import File",
          End,
          MUIA_Family_Child,
            import_directory_menu = MenuitemObject, MUIA_Menuitem_Title, "Import Dir List",
          End,
          MUIA_Family_Child,
            quit_menu = MenuitemObject, MUIA_Menuitem_Title, "Quit",
          End,
      End, // Project Menu Object
      MUIA_Family_Child, MenuObject,
        MUIA_Menu_Title, "Edit",
          MUIA_Family_Child,
            copy_menu = MenuitemObject, MUIA_Menuitem_Title, "Copy",
          End,
          MUIA_Family_Child,
            copy_matches_menu = MenuitemObject, MUIA_Menuitem_Title, "Copy Matches",
          End,
          MUIA_Family_Child,
            paste_menu = MenuitemObject, MUIA_Menuitem_Title, "Paste",
          End,
      End, // Edit Menu Object
    End, // End MenustripObject
    SubWindow, window = WindowObject,
      MUIA_Window_Title, "Patterns",
      MUIA_Window_ID, MAKE_ID('T','I','T','L'),
      MUIA_Window_AppWindow, TRUE,
        WindowContents,
        VGroup,
        Child, HGroup,
        Child, glob_text = BetterStringObject, TextFrame, End,
        Child, glob_button = SimpleButton("GLOB IT!"), End,
        Child, search_text_editor = TextEditorObject, StringFrame, MUIA_CycleChain, 1,
//        MUIA_TextEditor_ExportHook, MUIV_TextEditor_ExportHook_NoStyle,
        End,
      End,
    End,
  End;
}

/**
 * Configure MUI hooks and bind to window controls
 */
void ConfigureAndBind() {
  static struct Hook GlobItClickHook, QuitMenuHook, ImportFileMenuHook, ImportDirectoryMenuHook, CopyMatchesMenuHook;
  GlobItClickHook.h_Entry = (HOOKFUNC)OnGlobItClick;
  QuitMenuHook.h_Entry = (HOOKFUNC)OnQuitMenu;
  ImportFileMenuHook.h_Entry = (HOOKFUNC)OnImportFileMenu;
  ImportDirectoryMenuHook.h_Entry = (HOOKFUNC)OnImportDirectoryMenu;
  CopyMatchesMenuHook.h_Entry = (HOOKFUNC)OnCopyMatchesMenu;

  // Window Controls
  DoMethod(window,MUIM_Notify,MUIA_Window_CloseRequest,TRUE, app,2,MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);

  // Menu Items
  DoMethod(quit_menu, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, app, 2, MUIM_CallHook,  &QuitMenuHook);
  DoMethod(import_file_menu, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, app, 2, MUIM_CallHook,  &ImportFileMenuHook);
  DoMethod(import_directory_menu, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, app, 2, MUIM_CallHook,  &ImportDirectoryMenuHook);

  DoMethod(copy_matches_menu, MUIM_Notify, MUIA_Menuitem_Trigger, MUIV_EveryTime, app, 2, MUIM_CallHook,  &CopyMatchesMenuHook);

  // Buttons
  DoMethod(glob_button, MUIM_Notify, MUIA_Pressed, FALSE, glob_button, 2, MUIM_CallHook, &GlobItClickHook);
}

/*************************************************************
 * On X Event Handlers
 *************************************************************/

void OnGlobItClick(void) {
  STRPTR pattern_string; // Place to hold the pattern the user is looking for
  STRPTR search_string; // Place to hold the plain text (no markup) version of the text we are searching

  // let's populate our needle and haystack
  search_string = ClearTextEditorMarkup(search_text_editor);
  get(glob_text, MUIA_String_Contents, &pattern_string);

  // Clear the next editor
  DoMethod(search_text_editor, MUIM_TextEditor_ClearText);
  ForEachLine(search_string, &ProcessGlobLine, pattern_string);
}

void OnQuitMenu(void) {
  DoMethod(app, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
}

void PushFileInfoToEditorContent(struct FileInfoBlock* file_info) {
  AppendControl(file_info->fib_FileName, search_text_editor);
//  SetTextEditorContent(file_info->fib_FileName, search_text_editor);
}

void OnImportDirectoryMenu(void) {
  LONG length;
  STRPTR buffer;
  UBYTE fname[255];

  if(!DialogGetFileSystemPath(DIALOG_DIRECTORY, fname)) {
    return;
  }

  if(ReadDirListIntoBuffer(fname, &buffer, &length)) {
    SetTextEditorContent(buffer, search_text_editor);
  }
//  DoMethod(search_text_editor, MUIM_TextEditor_ClearText);
//  ForEachReadDirList(fname, PushFileInfoToEditorContent);
}

void OnImportFileMenu(void) {
  LONG length;
  STRPTR buffer;
  UBYTE fname[255];

  if(!DialogGetFileSystemPath(DIALOG_FILE, fname)) {
    return;
  }
//  DialogGetFileSystemPath(DIALOG_DIRECTORY, fname);

  if(ReadFileIntoBuffer(fname, &buffer, &length)) {
    SetTextEditorContent(buffer, search_text_editor);
    FreeMem(buffer, length);
  }
}

void OnCopyMatchesMenu(void) {
  STRPTR matches;
  STRPTR exported_text = (STRPTR)DoMethod(search_text_editor, MUIM_TextEditor_ExportText);

  GetHighlights(&matches, exported_text);
  printf("matches:\n%s\n", matches);
  FreeVec(matches);
}
/*************************************************************
 * End On X Event Handlers
 *************************************************************/

BOOL DialogGetFileSystemPath(enum FileDialogType type, STRPTR fname) {
  struct FileRequester *request;
  APTR intuition_window;
  LONG length;

  fname[0] = '\0';

  GetAttr(MUIA_Window_Window, window, &intuition_window);
  if(type == DIALOG_FILE) {
    request = (struct FileRequester *) AllocAslRequestTags(ASL_FileRequest,
                                                           ASL_Hail, "Open File",
                                                           ASLFR_DoPatterns, TRUE,
                                                           ASLFR_InitialPattern, "#?.txt",
                                                           (struct TagItem *) TAG_DONE);
  } else if (type == DIALOG_DIRECTORY) {
    request = (struct FileRequester *) AllocAslRequestTags(ASL_FileRequest,
                                                           ASL_Hail, "List Directory",
                                                           ASLFR_DrawersOnly, TRUE,
                                                           (struct TagItem *) TAG_DONE);
  } else {
    return FALSE;
  }

  if (AslRequestTags(request,
                     ASLFR_Window, intuition_window,
                     ASLFR_PositiveText, "Open",
                     (struct TagItem *)TAG_DONE)) {
//    strcat( fname, request->rf_Dir);
    CopyMem(request->rf_Dir, fname, strlen(request->rf_Dir) + 1);
    length = strlen(fname);
    if(length > 0 && fname[length-1] != (UBYTE) ':') {
      strcat(fname, "/");
    }
    strcat(fname, request->rf_File);

    if (request) {
      FreeAslRequest(request);
    }
    return TRUE;
  }
  else {
    return FALSE;
  }
}
