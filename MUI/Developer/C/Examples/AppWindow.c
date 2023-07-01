#include "demo.h"


/*
** App message callback hook. Note that the object given here
** is the object that called the hook, i.e. the one that got
** the icon(s) dropped on it.
*/

SAVEDS ASM LONG AppMsgFunc(REG(a2) APTR obj, REG(a1) struct AppMessage **x)
{
	struct WBArg *ap;
	struct AppMessage *amsg = *x;
	int i;
	static char buf[256];
	char *b=buf;

	for (ap=amsg->am_ArgList,i=0;i<amsg->am_NumArgs;i++,ap++)
	{
		NameFromLock(ap->wa_Lock,buf,sizeof(buf));
		AddPart(buf,ap->wa_Name,sizeof(buf));
		DoMethod(obj,MUIM_List_Insert,&b,1,MUIV_List_Insert_Bottom);
	}

	return(0);
}


/*
** Having a function instead of a macro saves some code.
*/

APTR MakeLV(VOID)
{
	return(
		ListviewObject,
			MUIA_Listview_Input, FALSE,
			MUIA_Listview_List , ListObject,
				ReadListFrame,
				MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
				MUIA_List_DestructHook , MUIV_List_DestructHook_String ,
				End,
			End
	);
}


int main(int argc,char *argv[])
{
	APTR app,window;
	APTR lv1,lv2,lv3;
	static const struct Hook AppMsgHook = { { NULL,NULL },(VOID *)AppMsgFunc,NULL,NULL };

	init();

	app = ApplicationObject,
		MUIA_Application_Title      , "AppWindowDemo",
		MUIA_Application_Version    , "$VER: AppWindowDemo 19.5 (12.02.97)",
		MUIA_Application_Copyright  , "�1992/93, Stefan Stuntz",
		MUIA_Application_Author     , "Stefan Stuntz",
		MUIA_Application_Description, "Show AppWindow Handling",
		MUIA_Application_Base       , "APPWINDOWDEMO",

		SubWindow, window = WindowObject,
			MUIA_Window_Title    , "Drop icons on me!",
			MUIA_Window_ID       , MAKE_ID('A','P','P','W'),
			MUIA_Window_AppWindow, TRUE,

			WindowContents, VGroup,
				Child, HGroup,
					Child, lv1 = MakeLV(),
					Child, lv2 = MakeLV(),
					End,
				Child, lv3 = MakeLV(),
				End,

			End,

		End;

	if (!app)
		fail(app,"Failed to create Application.");

	DoMethod(window,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
		app,2,MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);


/*
** Call the AppMsgHook when an icon is dropped on a listview.
*/

	DoMethod(lv1,MUIM_Notify,MUIA_AppMessage,MUIV_EveryTime,
		lv1,3,MUIM_CallHook,&AppMsgHook,MUIV_TriggerValue);

	DoMethod(lv2,MUIM_Notify,MUIA_AppMessage,MUIV_EveryTime,
		lv2,3,MUIM_CallHook,&AppMsgHook,MUIV_TriggerValue);

	DoMethod(lv3,MUIM_Notify,MUIA_AppMessage,MUIV_EveryTime,
		lv3,3,MUIM_CallHook,&AppMsgHook,MUIV_TriggerValue);


/*
** When we're iconified, the object lv3 shall receive the
** messages from icons dropped on our app icon.
*/

	set(app,MUIA_Application_DropObject,lv3);


/*
** This is the ideal input loop for an object oriented MUI application.
** Everything is encapsulated in classes, no return ids need to be used,
** we just check if the program shall terminate.
** Note that MUIM_Application_NewInput expects sigs to contain the result
** from Wait() (or 0). This makes the input loop significantly faster.
*/

	set(window,MUIA_Window_Open,TRUE);

	{
		ULONG sigs = 0;

		while (DoMethod(app,MUIM_Application_NewInput,&sigs) != MUIV_Application_ReturnID_Quit)
		{
			if (sigs)
			{
				sigs = Wait(sigs | SIGBREAKF_CTRL_C);
				if (sigs & SIGBREAKF_CTRL_C) break;
			}
		}
	}

	set(window,MUIA_Window_Open,FALSE);


/*
** Shut down...
*/

	fail(app,NULL);
}
