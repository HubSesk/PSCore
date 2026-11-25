class PS_ManualMarkerClass : GenericEntityClass
{

}

// Create custom widget on map every time player open map
/*
	Position and rotation -- Got from entity on map. 
	ImageSet -- Can be used any imageSet, but GM use only registered list of custom markers
	ImageSetGlow -- Can be empty
	QuadName -- Name of the Quad used for imageSet AND imageSetGlow
	Color -- Color of marker icon
	Size -- Size in meter or pixels, m_bUseWorldScale = 1 mean in meter else in pixels (Default layout size is 1920x1080 pixels)
	Description -- Multiline marker description show on hover
	Visibility for factions -- Which factions this marker been show, it's use PLAYER faction, not entity.
	
	Widget update position, size and rotation every frame when present in screen.
*/
class PS_ManualMarker : GenericEntity
{
	// Attributes for editing through workbench TODO: proper description
	[Attribute("{D17288006833490F}UI/Textures/Icons/icons_wrapperUI-32.imageset")]
	protected ResourceName m_sImageSet;
	[Attribute("")]
	protected ResourceName m_sImageSetGlow;
	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
	ref Color m_MarkerColor;
	[Attribute("empty")]
	protected string m_sQuadName;
	[Attribute("5.0")]
	protected float m_fWorldSize;
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBoxMultiline)]
	protected string m_sDescription;
	[Attribute("true")]
	bool m_bUseWorldScale;
		
	[Attribute("")]
	ref array<FactionKey> m_aVisibleForFactions;
	[Attribute("")]
	bool m_bVisibleForEmptyFaction;
	
	[Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_EGameModeState))]
	ref array<SCR_EGameModeState> m_aHideOnGameModeStates;
	
	[Attribute("")]
	int m_iZOrder;
	
	[Attribute("")]
	bool m_bShowForAnyFaction;
	
	[Attribute("false", desc: "Track the parent inventory item; requires an independent, non-streamable RplComponent.")]
	protected bool m_bTrackInventory;

	// Internal variables
	Widget m_wRoot;
	SCR_MapEntity m_MapEntity;
	PS_ManualMarkerComponent m_hManualMarkerComponent;
	protected ResourceName m_sMarkerPrefab = "{52CA8FF5F56C6F31}UI/layouts/Map/ManualMapMarkerBase.layout";
	
	[RplProp(onRplName: "OnRpl_ServerPos")]
	protected vector m_vServerPos;

	[RplProp(onRplName: "OnRpl_ServerPos")]
	protected float m_fServerYaw;

	[RplProp(onRplName: "OnRpl_ServerPos")]
	protected bool m_bHasServerPos;

	protected static ref array<PS_ManualMarker> s_aInstances = {};
	protected static BaseWorld s_RegistryWorld;

	//! Returns active PS_ManualMarker instances in the current world (used by external observers such as replay).
	static void GetInstances(notnull array<PS_ManualMarker> outInstances)
	{
		outInstances.Clear();
		if (!s_aInstances)
			return;

		BaseWorld currentWorld = GetGame().GetWorld();
		foreach (PS_ManualMarker instance : s_aInstances)
		{
			if (!instance || instance.IsDeleted())
				continue;

			if (currentWorld && instance.GetWorld() != currentWorld)
				continue;

			outInstances.Insert(instance);
		}
	}

	protected void RegisterInstance()
	{
		BaseWorld world = GetWorld();
		if (world && world != s_RegistryWorld)
		{
			s_aInstances.Clear();
			s_RegistryWorld = world;
		}

		if (!s_aInstances.Contains(this))
			s_aInstances.Insert(this);
	}

	void GetWorldPosAndYaw(out vector pos, out float yaw)
	{
		if (m_bHasServerPos)
		{
			pos = m_vServerPos;
			yaw = m_fServerYaw;
			return;
		}

		pos = GetOrigin();
		yaw = GetYawPitchRoll()[0];
	}

	// Get/Set Broadcast
	// Setters must be call from autority (usualy server)
	string GetImageSet()
	{
		return m_sImageSet;
	}
	void SetImageSet(string imageSet)
	{
		RPC_SetImageSet(imageSet);
		Rpc(RPC_SetImageSet, imageSet);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetImageSet(string imageSet)
	{
		m_sImageSet = imageSet;
		
		// Update "On fly" if map open and marker exist
		if (m_hManualMarkerComponent) m_hManualMarkerComponent.SetImage(m_sImageSet, m_sQuadName);
	}
	
	string GetImageSetGlow()
	{
		return m_sImageSetGlow;
	}
	void SetImageSetGlow(string imageSetGlow)
	{
		RPC_SetImageSetGlow(imageSetGlow);
		Rpc(RPC_SetImageSetGlow, imageSetGlow);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetImageSetGlow(string imageSetGlow)
	{
		m_sImageSetGlow = imageSetGlow;
		
		// Update "On fly" if map open and marker exist
		if (m_hManualMarkerComponent) m_hManualMarkerComponent.SetImageGlow(m_sImageSetGlow, m_sQuadName);
	}
	
	string GetQuadName()
	{
		return m_sQuadName;
	}
	void SetQuadName(string quadName)
	{
		RPC_SetQuadName(quadName);
		Rpc(RPC_SetQuadName, quadName);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetQuadName(string quadName)
	{
		m_sQuadName = quadName;
		
		// Update "On fly" if map open and marker exist
		if (m_hManualMarkerComponent) 
		{
			m_hManualMarkerComponent.SetImage(m_sImageSet, m_sQuadName);
			m_hManualMarkerComponent.SetImageGlow(m_sImageSetGlow, m_sQuadName);
		}
	}
	
	Color GetColor()
	{
		return m_MarkerColor;
	}
	void SetColor(Color color)
	{
		RPC_SetColor_ByInt(color.PackToInt());
		Rpc(RPC_SetColor_ByInt, color.PackToInt());
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetColor_ByInt(int colorI)
	{
		Color color = Color.FromInt(colorI);
		m_MarkerColor = color;
		
		// Update "On fly" if map open and marker exist
		if (m_hManualMarkerComponent) m_hManualMarkerComponent.SetColor(m_MarkerColor);
	}
	
	float GetSize()
	{
		return m_fWorldSize;
	}
	void SetSize(float worldSize)
	{
		RPC_SetSize(worldSize);
		Rpc(RPC_SetSize, worldSize);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetSize(float worldSize)
	{
		m_fWorldSize = worldSize;
		
		if (m_wRoot)
			UpdateMapWidget();
	}
	
	string GetDescription()
	{
		return m_sDescription;
	}
	void SetDescription(string description)
	{
		RPC_SetDescription(description);
		Rpc(RPC_SetDescription, description);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetDescription(string description)
	{
		m_sDescription = description;
		
		// Update "On fly" if map open and marker exist
		if (m_hManualMarkerComponent) m_hManualMarkerComponent.SetDescription(m_sDescription);
	}
	
	bool GetUseWorldScale()
	{
		return m_bUseWorldScale;
	}
	void SetUseWorldScale(bool useWorldScale)
	{
		RPC_SetUseWorldScale(useWorldScale);
		Rpc(RPC_SetUseWorldScale, useWorldScale);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetUseWorldScale(bool useWorldScale)
	{
		m_bUseWorldScale = useWorldScale;
		
		if (m_wRoot)
			UpdateMapWidget();
	}
	
	bool GetVisibleForEmptyFaction()
	{
		return m_bVisibleForEmptyFaction;
	}
	void SetVisibleForEmptyFaction(bool visibleForEmptyFaction)
	{
		RPC_SetVisibleForEmptyFaction(visibleForEmptyFaction);
		Rpc(RPC_SetVisibleForEmptyFaction, visibleForEmptyFaction);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetVisibleForEmptyFaction(bool visibleForEmptyFaction)
	{
		m_bVisibleForEmptyFaction = visibleForEmptyFaction;
		
		// Update "On fly" show/hide marker, if map currently open
		if (!m_MapEntity) return;
		if (!m_MapEntity.IsOpen()) return;
		if (IsCurrentFactionVisibility())
		{
			if (!m_wRoot) CreateMapWidget(m_MapEntity.GetMapConfig());
		} else {
			if (m_wRoot) DeleteMapWidget(m_MapEntity.GetMapConfig());
		}
	}
	
	bool GetVisibleForFaction(FactionKey factionKey)
	{
		if (m_bShowForAnyFaction) return true;
		return m_aVisibleForFactions.Contains(factionKey);
	}

	void SetVisibleForFaction(Faction faction, bool visible)
	{
		RPC_SetVisibleForFaction_ByKey(faction.GetFactionKey(), visible);
		Rpc(RPC_SetVisibleForFaction_ByKey, faction.GetFactionKey(), visible);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetVisibleForFaction_ByKey(FactionKey factionKey, bool visible)
	{
		Faction faction = GetGame().GetFactionManager().GetFactionByKey(factionKey);
		if (visible)
			{if (!GetVisibleForFaction(faction.GetFactionKey())) m_aVisibleForFactions.Insert(faction.GetFactionKey());}
		else
			{if (GetVisibleForFaction(faction.GetFactionKey())) m_aVisibleForFactions.RemoveItem(faction.GetFactionKey());}
		
		// Update "On fly" show/hide marker, if map currently open
		if (!m_MapEntity) return;
		if (!m_MapEntity.IsOpen()) return;
		if (IsCurrentFactionVisibility())
		{
			if (!m_wRoot) CreateMapWidget(m_MapEntity.GetMapConfig());
		} else {
			if (m_wRoot) DeleteMapWidget(m_MapEntity.GetMapConfig());
		}
	}
	
	// Self functions
	protected void UpdateMapWidget()
	{
		if (!m_wRoot || !m_MapEntity)
			return;
		
		// Get screen position
		float wX, wY, screenX, screenY, screenXEnd, screenYEnd;
		float trackedYaw = GetYawPitchRoll()[0];
		vector worldPosition = GetOrigin();
		if (m_bHasServerPos)
		{
			worldPosition = m_vServerPos;
			trackedYaw = m_fServerYaw;
		}
		wX = worldPosition[0];
		wY = worldPosition[2];
		m_MapEntity.WorldToScreen(wX, wY, screenX, screenY, true);
		m_MapEntity.WorldToScreen(wX + m_fWorldSize, wY + m_fWorldSize, screenXEnd, screenYEnd, true);
		
		// Scale to workspace
		float screenXD = GetGame().GetWorkspace().DPIUnscale(screenX);
		float screenYD = GetGame().GetWorkspace().DPIUnscale(screenY);
		float sizeXD = m_fWorldSize;
		float sizeYD = m_fWorldSize;
		if (m_bUseWorldScale) // Calculate world size if need
		{
			sizeXD = GetGame().GetWorkspace().DPIUnscale(screenXEnd - screenX);
			sizeYD = GetGame().GetWorkspace().DPIUnscale(screenY - screenYEnd); // Y flip
		}
		sizeYD *= m_hManualMarkerComponent.GetYScale();
		
		// Update widget
		// Since every default direction marcers turned to right, -90° added to entity rotation
		m_hManualMarkerComponent.SetSlot(screenXD, screenYD, sizeXD, sizeYD, trackedYaw - 90);
	}

	protected void OnRpl_ServerPos()
	{
		if (m_wRoot)
			UpdateMapWidget();
	}

	// PostFrame runs after SCR_MapEntity.EOnFrame so map zoom/pan values are from the current frame.
	override protected void EOnPostFrame(IEntity owner, float timeSlice)
	{
		UpdateMapWidget();
	}

	protected void OnMapZoom(float zoomVal)
	{
		UpdateMapWidget();
	}

	protected void OnMapPan(float panX, float panY, bool adjustedPan)
	{
		UpdateMapWidget();
	}
	
	override protected void EOnInit(IEntity owner)
	{		
		RegisterInstance();

		if (m_aVisibleForFactions == null)
			m_aVisibleForFactions = new array<FactionKey>();
		
		if (m_bTrackInventory && Replication.IsServer())
		{
			GetGame().GetCallqueue().CallLater(UpdateTrackedPosition_S, 0, false);
			GetGame().GetCallqueue().CallLater(UpdateTrackedPosition_S, 1000, true);
		}

		m_MapEntity = SCR_MapEntity.GetMapInstance();
		if (!m_MapEntity)
			return;

		ScriptInvokerBase<MapConfigurationInvoker> onMapOpen = m_MapEntity.GetOnMapOpen();
		ScriptInvokerBase<MapConfigurationInvoker> onMapClose = m_MapEntity.GetOnMapClose();
		
		// Every time map open/close add/remove widget
		onMapOpen.Insert(CreateMapWidget);
		onMapClose.Insert(DeleteMapWidget);

		// Wait until derived marker classes have completed initialization.
		GetGame().GetCallqueue().CallLater(CreateMapWidgetIfOpen_C, 0, false);
	}
	
	bool IsCurrentFactionVisibility()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return true;
		
		if (m_aHideOnGameModeStates.Contains(gameMode.GetState()))
			return false;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return true; // Somehow manager lost, show marker
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return true; // Somehow player controller lost, show marker
		
		SCR_PlayerFactionAffiliationComponent playerFactionAffiliationComponent = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!playerFactionAffiliationComponent)
			return true; // Somehow player faction component lost, show marker
		
		Faction faction = playerFactionAffiliationComponent.GetAffiliatedFaction();
		if (faction == null)
		{
			return m_bVisibleForEmptyFaction;
		}
		
		// Check is player faction in visibility list
		return GetVisibleForFaction(faction.GetFactionKey());
	}
	
	// Create new widget in map frame widget
	void CreateMapWidget(MapConfiguration mapConfig)
	{
		// If marker already exists ignore
		if (m_wRoot)
			return;
		
		// Faction visibility check
		if (!IsCurrentFactionVisibility())
			return;
		
		// Get map frame
		Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
		if (!mapFrame) mapFrame = m_MapEntity.GetMapMenuRoot();
		if (!mapFrame) return; // Somethig gone wrong
		
		// Create and init marker
		m_wRoot = GetGame().GetWorkspace().CreateWidgets(m_sMarkerPrefab, mapFrame);
		m_wRoot.SetZOrder(m_iZOrder);
		m_hManualMarkerComponent = PS_ManualMarkerComponent.Cast(m_wRoot.FindHandler(PS_ManualMarkerComponent));
		m_hManualMarkerComponent.SetImage(m_sImageSet, m_sQuadName);
		m_hManualMarkerComponent.SetImageGlow(m_sImageSetGlow, m_sQuadName);
		m_hManualMarkerComponent.SetDescription(m_sDescription);
		m_hManualMarkerComponent.SetColor(m_MarkerColor);
		m_hManualMarkerComponent.OnMouseLeave(null, null, 0, 0);
		
		// Sync immediately and keep updated on map zoom/pan and active entity post-frame.
		UpdateMapWidget();
		SetEventMask(EntityEvent.POSTFRAME);

		ScriptInvokerFloat onMapZoom = SCR_MapEntity.GetOnMapZoom();
		if (onMapZoom)
			onMapZoom.Insert(OnMapZoom);

		ScriptInvokerBase<ScriptInvokerFloat2Bool> onMapPan = SCR_MapEntity.GetOnMapPan();
		if (onMapPan)
			onMapPan.Insert(OnMapPan);
	}
	
	protected void UnsubscribeMapTransformEvents()
	{
		ScriptInvokerFloat onMapZoom = SCR_MapEntity.GetOnMapZoom();
		if (onMapZoom)
			onMapZoom.Remove(OnMapZoom);

		ScriptInvokerBase<ScriptInvokerFloat2Bool> onMapPan = SCR_MapEntity.GetOnMapPan();
		if (onMapPan)
			onMapPan.Remove(OnMapPan);
	}

	// Delete map widget
	void DeleteMapWidget(MapConfiguration mapConfig)
	{
		// Remove widget if exists
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
		
		// Forget
		m_wRoot = null;
		m_hManualMarkerComponent = null;
		
		// Disable every frame updating and map transform listeners
		ClearEventMask(EntityEvent.POSTFRAME);
		UnsubscribeMapTransformEvents();
	}
	
	void PS_ManualMarker(IEntitySource src, IEntity parent)
	{
		// Enable init event
		SetEventMask(EntityEvent.INIT);
	}

	/**
	 * @brief Отписка от инвокеров карты и удаление виджета при удалении маркера
	 * @issue BUG-64
	 * @cause Отсутствие деструктора приводило к вызову CreateMapWidget/DeleteMapWidget на удалённом объекте и утечке m_wRoot
	 * @solution Отписка от onMapOpen/onMapClose и удаление m_wRoot из иерархии в деструкторе
	 */
	void ~PS_ManualMarker()
	{
		if (s_aInstances)
			s_aInstances.RemoveItem(this);

		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(UpdateTrackedPosition_S);
			GetGame().GetCallqueue().Remove(CreateMapWidgetIfOpen_C);
			GetGame().GetCallqueue().Remove(FactionsInit);
		}

		if (m_wRoot)
		{
			m_wRoot.RemoveFromHierarchy();
			m_wRoot = null;
		}
		m_hManualMarkerComponent = null;

		UnsubscribeMapTransformEvents();

		if (m_MapEntity)
		{
			ScriptInvokerBase<MapConfigurationInvoker> onMapOpen = m_MapEntity.GetOnMapOpen();
			if (onMapOpen)
				onMapOpen.Remove(CreateMapWidget);

			ScriptInvokerBase<MapConfigurationInvoker> onMapClose = m_MapEntity.GetOnMapClose();
			if (onMapClose)
				onMapClose.Remove(DeleteMapWidget);
		}
	}
	
	// The server has the inventory hierarchy even when the item is absent on clients.
	protected void UpdateTrackedPosition_S()
	{
		if (!Replication.IsServer())
			return;

		IEntity item = GetParent();
		if (!item || item.IsDeleted())
			return;

		InventoryItemComponent inventoryItem = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!inventoryItem)
			return;

		IEntity carrier = item;
		InventoryStorageSlot slot = inventoryItem.GetParentSlot();
		if (slot && slot.GetOwner())
			carrier = slot.GetOwner().GetRootParent();

		vector position = carrier.GetOrigin();
		float yaw = carrier.GetYawPitchRoll()[0];
		if (m_bHasServerPos && position == m_vServerPos && yaw == m_fServerYaw)
			return;

		m_vServerPos = position;
		m_fServerYaw = yaw;
		m_bHasServerPos = true;
		Replication.BumpMe();

		if (m_wRoot)
			UpdateMapWidget();
	}

	protected void CreateMapWidgetIfOpen_C()
	{
		if (m_MapEntity && m_MapEntity.IsOpen() && m_MapEntity.GetMapMenuRoot())
			CreateMapWidget(m_MapEntity.GetMapConfig());
	}

	// JIP Replication
	override bool RplSave(ScriptBitWriter writer)
	{
		// Pack every changeable variable
		writer.WriteString(m_sImageSet);
		writer.WriteString(m_sImageSetGlow);
		writer.WriteString(m_sQuadName);
		writer.WriteInt(m_MarkerColor.PackToInt());
		writer.WriteFloat(m_fWorldSize);
		writer.WriteString(m_sDescription);
		writer.WriteBool(m_bUseWorldScale);
		writer.WriteBool(m_bVisibleForEmptyFaction);
		
		string factions = "";
		foreach (FactionKey factionKey: m_aVisibleForFactions)
		{
			if (factions != "") factions += ",";
			factions += factionKey;
		}
		writer.WriteString(factions);
		
		return true;
	}
	override bool RplLoad(ScriptBitReader reader)
	{
		// Unpack every changeable variable
		reader.ReadString(m_sImageSet);
		reader.ReadString(m_sImageSetGlow);
		reader.ReadString(m_sQuadName);
		int colorI; // This break flow, where is Color serealization? :(
		reader.ReadInt(colorI);
		m_MarkerColor = Color.FromInt(colorI);
		reader.ReadFloat(m_fWorldSize);
		reader.ReadString(m_sDescription);
		reader.ReadBool(m_bUseWorldScale);
		reader.ReadBool(m_bVisibleForEmptyFaction);
		
		string factions;
		reader.ReadString(factions);
		GetGame().GetCallqueue().CallLater(FactionsInit, 0, false, factions);
		
		return true;
	}
	
	void FactionsInit(string factions)
	{
		array<string> outTokens = new array<string>();
		factions.Split(",", outTokens, false);
		foreach (FactionKey factionKey: outTokens)
		{
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			Faction faction = factionManager.GetFactionByKey(factionKey);
			if (faction)
				m_aVisibleForFactions.Insert(faction.GetFactionKey());
		}
		CreateMapWidgetIfOpen_C();
	}
}
