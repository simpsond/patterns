#include "demo.h"

/*****************************************************************************
** This is the Mousepower custom class, a sub class of Levelmeter.mui.
** It is quite simple and does nothing but add some input capabilities
** to its super class by implementing MUIM_HandleInput.
** Don't be afraid of writing sub classes!
******************************************************************************/

struct MousepowerData
{
	WORD Decrease;
	WORD MouseX;
	WORD MouseY;
	WORD Direction;
};

#define MUIA_Mousepower_Direction ((TAG_USER | ( 1 << 16)) | 0x0001)

SAVEDS ASM ULONG MousepowerDispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg)
{
	switch (msg->MethodID)
	{
		case OM_NEW:
		{
			if (obj=(Object *)DoSuperMethodA(cl,obj,msg))
			{
				struct MousepowerData *data = INST_DATA(cl,obj);
				data->MouseX    = -1;
				data->Direction = GetTagData(MUIA_Mousepower_Direction,0,((struct opSet *)msg)->ops_AttrList);
				set(obj,MUIA_Numeric_Max,1000);
			}
			return((ULONG)obj);
		}
		break;

		case MUIM_Setup:
		{
			struct MousepowerData *data = INST_DATA(cl,obj);
			if (!DoSuperMethodA(cl,obj,msg))
				return(FALSE);
			data->MouseX = -1;
			set(obj,MUIA_Numeric_Max,1000);
			MUI_RequestIDCMP(obj,IDCMP_MOUSEMOVE|IDCMP_INTUITICKS|IDCMP_INACTIVEWINDOW);
			return(TRUE);
		}
		break;

		case MUIM_Cleanup:
		{
			MUI_RejectIDCMP(obj,IDCMP_MOUSEMOVE|IDCMP_INTUITICKS|IDCMP_INACTIVEWINDOW);
			return(DoSuperMethodA(cl,obj,msg));
		}
		break;

		case MUIM_HandleInput:
		{
			struct MUIP_HandleInput *m = (APTR)msg;
			struct MousepowerData *data = INST_DATA(cl,obj);

			if (m->imsg)
			{
				if (m->imsg->Class==IDCMP_MOUSEMOVE)
				{
					if (data->MouseX!=-1)
					{
						LONG delta;

						switch (data->Direction)
						{
							case 1 : delta = abs(data->MouseX - m->imsg->MouseX) * 2; break;
							case 2 : delta = abs(data->MouseY - m->imsg->MouseY) * 2; break;
							default: delta = abs(data->MouseX - m->imsg->MouseX) + abs(data->MouseY - m->imsg->MouseY); break;
						}
						if (data->Decrease>0) data->Decrease--;
						DoMethod(obj,MUIM_Numeric_Increase,delta/10);
					}
					data->MouseX = m->imsg->MouseX;
					data->MouseY = m->imsg->MouseY;
				}
				else if (m->imsg->Class==IDCMP_INTUITICKS)
				{
					DoMethod(obj,MUIM_Numeric_Decrease,data->Decrease*data->Decrease);
					if (data->Decrease<50) data->Decrease++;
				}
				else if (m->imsg->Class==IDCMP_INACTIVEWINDOW)
				{
					set(obj,MUIA_Numeric_Value,0);
				}
			}

			return(0);
		}
		break;
	}

	return(DoSuperMethodA(cl,obj,msg));
}



/*****************************************************************************
** This is the Rating custom class, a sub class of Slider.mui.
** It shows how to override the MUIM_Numeric_Stringify method
** to implement custom displays in a slider gadget. Nothing
** easier than that... :-)
******************************************************************************/

struct RatingData
{
	char buf[20];
};

SAVEDS ASM ULONG RatingDispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg)
{
	if (msg->MethodID==MUIM_Numeric_Stringify)
	{
		struct RatingData *data = INST_DATA(cl,obj);
		struct MUIP_Numeric_Stringify *m = (APTR)msg;

		if (m->value==0)
		{
			strcpy(data->buf,"You're kidding!");
		}
		else if (m->value==100)
		{
			strcpy(data->buf,"It's magic!");
		}
		else
		{
			static const STRPTR ratings[] = { ":-((",":-(",":-|",":-)",":-))" };
			LONG r = DoMethod(obj,MUIM_Numeric_ValueToScale,0,sizeof(ratings)/sizeof(STRPTR)-1);
			sprintf(data->buf,"%3ld points. %s",m->value,(char *)ratings[r]);
		}
		return((ULONG)data->buf);
	}
	return(DoSuperMethodA(cl,obj,msg));
}



/*****************************************************************************
** A time slider custom class. Just like with the Rating class, we override
** the MUIM_Numeric_Stringify method. Wow... our classes get smaller and 
** smaller. This one only has about 10 lines of C code. :-)
** Note that we can use this TimeDispatcher as subclass of any of
** MUI's numeric classes. In Slidorama, we create a Timebutton class
** from MUIC_Numericbutton and Timeslider class for MUIC_Slider with
** the same dispatcher function.
******************************************************************************/

struct TimeData
{
	char buf[16];
};

SAVEDS ASM ULONG TimeDispatcher(REG(a0) struct IClass *cl,REG(a2) Object *obj,REG(a1) Msg msg)
{
	if (msg->MethodID==MUIM_Numeric_Stringify)
	{
		struct TimeData *data = INST_DATA(cl,obj);
		struct MUIP_Numeric_Stringify *m = (APTR)msg;
		sprintf(data->buf,"%02ld:%02ld",m->value/60,m->value%60);
		return((ULONG)data->buf);
	}
	return(DoSuperMethodA(cl,obj,msg));
}



/*****************************************************************************
** Main Program
******************************************************************************/

static struct MUI_CustomClass *MousepowerClass;
static struct MUI_CustomClass *RatingClass;
static struct MUI_CustomClass *TimebuttonClass;
static struct MUI_CustomClass *TimesliderClass;

static VOID CleanupClasses(VOID)
{
	if (MousepowerClass) MUI_DeleteCustomClass(MousepowerClass);
	if (RatingClass    ) MUI_DeleteCustomClass(RatingClass);
	if (TimebuttonClass) MUI_DeleteCustomClass(TimebuttonClass);
	if (TimesliderClass    ) MUI_DeleteCustomClass(TimesliderClass);
}

static BOOL SetupClasses(VOID)
{
	MousepowerClass = MUI_CreateCustomClass(NULL,MUIC_Levelmeter,NULL,sizeof(struct MousepowerData),MousepowerDispatcher);
	RatingClass     = MUI_CreateCustomClass(NULL,MUIC_Slider,NULL,sizeof(struct RatingData),RatingDispatcher);
	TimesliderClass = MUI_CreateCustomClass(NULL,MUIC_Slider,NULL,sizeof(struct TimeData),TimeDispatcher);
	TimebuttonClass = MUI_CreateCustomClass(NULL,MUIC_Numericbutton,NULL,sizeof(struct TimeData),TimeDispatcher);

	if (MousepowerClass && RatingClass && TimebuttonClass && TimesliderClass)
		return(TRUE);

	CleanupClasses();
	return(FALSE);
}

int main(int argc,char *argv[])
{
	APTR app,window;

	init();

	if (!SetupClasses())
		fail(NULL,"Could not create custom classes.");

	app = ApplicationObject,
		MUIA_Application_Title      , "Slidorama",
		MUIA_Application_Version    , "$VER: Slidorama 19.5 (12.02.97)",
		MUIA_Application_Copyright  , "�1992-95, Stefan Stuntz",
		MUIA_Application_Author     , "Stefan Stuntz",
		MUIA_Application_Description, "Show different kinds of sliders",
		MUIA_Application_Base       , "SLIDORAMA",

		SubWindow, window = WindowObject,
			MUIA_Window_Title, "Slidorama",
			MUIA_Window_ID   , MAKE_ID('S','L','I','D'),

			WindowContents, VGroup,

				Child, HGroup,

					Child, VGroup, GroupSpacing(0), GroupFrameT("Knobs"),
						Child, VSpace(0),
						Child, ColGroup(6),
							GroupSpacing(0),
							Child, VSpace(0),
							Child, HSpace(4),
							Child, CLabel("1"),
							Child, CLabel("2"),
							Child, CLabel("3"),
							Child, CLabel("4"),
							Child, VSpace(2),
							Child, VSpace(2),
							Child, VSpace(2),
							Child, VSpace(2),
							Child, VSpace(2),
							Child, VSpace(2),
							Child, Label("Volume:"),
							Child, HSpace(4),
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Max, 64, MUIA_Numeric_Default, 64, End,
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Max, 64, MUIA_Numeric_Default, 64, End,
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Max, 64, MUIA_Numeric_Default, 64, End,
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Max, 64, MUIA_Numeric_Default, 64, End,
							Child, Label("Bass:"),
							Child, HSpace(4),
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Min, -100, MUIA_Numeric_Max, 100, End,
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Min, -100, MUIA_Numeric_Max, 100, End,
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Min, -100, MUIA_Numeric_Max, 100, End,
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Min, -100, MUIA_Numeric_Max, 100, End,
							Child, Label("Treble:"),
							Child, HSpace(4),
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Min, -100, MUIA_Numeric_Max, 100, End,
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Min, -100, MUIA_Numeric_Max, 100, End,
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Min, -100, MUIA_Numeric_Max, 100, End,
							Child, KnobObject, MUIA_CycleChain, 1, MUIA_Numeric_Min, -100, MUIA_Numeric_Max, 100, End,
							End,
						Child, VSpace(0),
						End,

					Child, VGroup,
						Child, VGroup, GroupFrameT("Levelmeter Displays"),
							Child, VSpace(0),
							Child, HGroup,
								Child, HSpace(0),
								Child, NewObject(MousepowerClass->mcc_Class,0,MUIA_Mousepower_Direction,1,MUIA_Levelmeter_Label,"Horizontal",TAG_DONE),
								Child, HSpace(0),
								Child, NewObject(MousepowerClass->mcc_Class,0,MUIA_Mousepower_Direction,2,MUIA_Levelmeter_Label,"Vertical"  ,TAG_DONE),
								Child, HSpace(0),
								Child, NewObject(MousepowerClass->mcc_Class,0,MUIA_Mousepower_Direction,0,MUIA_Levelmeter_Label,"Total"     ,TAG_DONE),
								Child, HSpace(0),
								End,
							Child, VSpace(0),
							End,
						Child, VGroup, GroupFrameT("Numeric Buttons"),
							Child, VSpace(0),
							Child, HGroup, GroupSpacing(0),
								Child, HSpace(0),
								Child, ColGroup(4), MUIA_Group_VertSpacing, 1,
									Child, VSpace(0),
									Child, CLabel("Left"),
									Child, CLabel("Right"),
									Child, CLabel("SPL"),
									Child, Label1("Low:"),
									Child, MUI_MakeObject(MUIO_NumericButton,NULL,0,100,"%3ld %%"),
									Child, MUI_MakeObject(MUIO_NumericButton,NULL,0,100,"%3ld %%"),
									Child, MUI_MakeObject(MUIO_NumericButton,NULL,30,99,"%2ld dB"),
									Child, Label1("Mid:"),
									Child, MUI_MakeObject(MUIO_NumericButton,NULL,0,100,"%3ld %%"),
									Child, MUI_MakeObject(MUIO_NumericButton,NULL,0,100,"%3ld %%"),
									Child, MUI_MakeObject(MUIO_NumericButton,NULL,30,99,"%2ld dB"),
									Child, Label1("High:"),
									Child, MUI_MakeObject(MUIO_NumericButton,NULL,0,100,"%3ld %%"),
									Child, MUI_MakeObject(MUIO_NumericButton,NULL,0,100,"%3ld %%"),
									Child, MUI_MakeObject(MUIO_NumericButton,NULL,30,99,"%2ld dB"),
									End,
								Child, HSpace(0),
								End,
							Child, VSpace(0),
							End,
						End,

					End,

				Child, VSpace(4),

				Child, ColGroup(2),
					Child, Label("Slidorama Rating:"),
					Child, NewObject(RatingClass->mcc_Class,0,
						MUIA_Numeric_Value,50,
						MUIA_CycleChain, 1,
						TAG_DONE),
					End,
				End,
			End,
		End;

	if (!app)
		fail(app,"Failed to create Application.");



	DoMethod(window,MUIM_Notify,MUIA_Window_CloseRequest,TRUE,
		app,2,MUIM_Application_ReturnID,MUIV_Application_ReturnID_Quit);

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

	MUI_DisposeObject(app);
	CleanupClasses();
	fail(NULL,NULL);
}
