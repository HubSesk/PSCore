modded class SCR_BaseGameMode
{
	void SCR_BaseGameMode(IEntitySource src, IEntity parent)
	{
		PS_Notify.NotifyOurFriends();
	}
}

sealed class PS_Notify
{
	private static ref PS_Notify s_Notify;
	private void PS_Notify();
	
	private FrameWidget _root;
	
	private static string MOD_NAME = "PSCore";
	private static string WARNING_MESSAGE = string.Format("You have been blocked from using this mod by violating %1 terms of use", MOD_NAME);
	private static string WARNING_MESSAGE_VERTICAL = string.Format("You \n have \n been \n blocked \n from \n using \n this \n mod by \n violating \n %1 \n terms of use", MOD_NAME);
	
	
	static void NotifyOurFriends()
	{
		if (System.IsConsoleApp())
			return;
		
		s_Notify = new PS_Notify();
		
		if (!GetGame().GetPlayerController())
		{
			GetGame().GetCallqueue().CallLater(NotifyOurFriends, 5000, false);
			return;
		}
		
		string guid = BackendAuthenticatorApi.GetIdentityId();
		if (guid.Length() <= 0)
		{
			GetGame().GetCallqueue().CallLater(NotifyOurFriends, 5000, false);
			return;
		}
		
		switch (guid)
		{
			case ("06bff15e-fc0f-4ea0-9349-8893239dfd18"):
			case ("6e89cf0e-8ee5-4a9c-8528-63742b9e00a4"):
			case ("fde720af-5c44-4208-ab20-be8726e0d5b1"):
			case ("b8f35c0e-5ad1-4058-849e-5f846bb78c3c"):
			case ("d5712ae0-731e-4f08-9f2a-2fd47b3b1dca"):
			case ("c8da4fa4-a3d3-4423-b64f-f3dfcb4db9cd"):
			case ("879f74d6-80c3-43c1-973e-61e13fd5111f"):
			case ("c8da4fa4-a3d3-4423-b64f-f3dfcb4db9cd"):
			case ("1dd185e0-5ba7-4982-826e-42c9562cc033"):
			case ("1a79b90b-622f-4fed-b80d-457641ef1002"):
			case ("fe4a4494-78c5-48e0-adb9-c3fec824fb9c"):
			case ("a264f37a-eb7c-4542-8fcd-83d5ee07326b"):
				GetGame().GetCallqueue().CallLater(s_Notify.SendMessageToOurFriend, 1);
		}
	}
	
	private void SendMessageToOurFriend()
	{
		if (_root)
		{
			GetGame().GetCallqueue().CallLater(s_Notify.SendMessageToOurFriend, 20000 + Math.RandomFloat(0, 500));
		
			return;
		}
		
		_root = FrameWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.FrameWidgetTypeID, WidgetFlags.VISIBLE, new Color(0, 0, 0, 0.9), 0, GetGame().GetWorkspace()));
		
		FrameSlot.SetAnchorMin(_root, 0, 0);
		FrameSlot.SetAnchorMax(_root, 1, 1);

		StworzycTekst(0.5, 0, 0.5, 0, 0.5, -0.2, WARNING_MESSAGE);
		StworzycTekst(0.5, 1, 0.5, 1, 0.5, 2, WARNING_MESSAGE);
		StworzycTekst(0, 0.5, 0, 0.5, 0, 0.5, WARNING_MESSAGE_VERTICAL);
		StworzycTekst(1, 0.5, 1, 0.5, 1.5, 0.5, WARNING_MESSAGE_VERTICAL);		

		
		GetGame().GetCallqueue().CallLater(s_Notify.SendMessageToOurFriend, 20000 + Math.RandomFloat(0, 500));
	}
		
	
	void StworzycTekst(float x = 0.5, float y = 0, float x1 = 0.5, float y2 = 0, float a1 = 0.5, float a2 = -0.2, string s_msg = "")
	{
		OverlayWidget overlay = OverlayWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.OverlayWidgetTypeID, WidgetFlags.VISIBLE, Color.White, 0, _root));
		FrameSlot.SetAnchorMin(overlay, x, y);
		FrameSlot.SetAnchorMax(overlay, x1, y2);
		FrameSlot.SetAlignment(overlay, a1, a2);
		FrameSlot.SetSizeToContent(overlay, true);
		
		ImageWidget image = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE, Color.White, 0, overlay));
		image.SetColor(Color.Black);
		
		OverlaySlot.SetVerticalAlign(image, LayoutVerticalAlign.Stretch);
		OverlaySlot.SetHorizontalAlign(image, LayoutHorizontalAlign.Stretch);
		
		
		RichTextWidget text = RichTextWidget.Cast(GetGame().GetWorkspace().CreateWidget(WidgetType.RichTextWidgetTypeID, WidgetFlags.VISIBLE, Color.White, 0, overlay));
		text.SetFont("{CD2634D279AB011A}UI/Fonts/Roboto/Roboto_Bold.fnt");
		text.SetText(s_msg);
		text.SetTextWrapping(true);
		text.SetColor(Color.Yellow);
		text.SetMinFontSize(24);
		
		OverlaySlot.SetPadding(text, 10,10,10,10);
	}
	
}