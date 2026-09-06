from pathlib import Path

script_path = Path('Scripts/Game/HOTASDebugger/HOTASSettingsTab.c')
layout_path = Path('UI/layouts/Menus/SettingsSubMenus/HOTASSettings.layout')

src = script_path.read_text(encoding='utf-8')

src = src.replace(
    '\tprotected bool m_bLoading;\n',
    '\tprotected bool m_bLoading;\n'
    '\tprotected Widget m_PreviewHost;\n'
    '\tprotected Widget m_PreviewSquare;\n'
    '\tprotected Widget m_PreviewSquareBackground;\n'
    '\tprotected Widget m_ScreenPreview;\n'
    '\tprotected Widget m_ScreenPreviewBackground;\n'
    '\tprotected Widget m_HudPositionPreview;\n'
)

src = src.replace(
    '\t\tSetupHudControls();\n\t\tm_bLoading = false;\n\t}\n',
    '\t\tSetupHudControls();\n'
    '\t\tSetupHudPositionPreview();\n'
    '\t\tm_bLoading = false;\n'
    '\t\tGetGame().GetCallqueue().CallLater(UpdateHudPositionPreview, 0, false);\n'
    '\t}\n',
    1
)

src = src.replace(
    '\t\tSyncHudControls();\n\t\tm_bLoading = false;\n\t}\n',
    '\t\tSyncHudControls();\n'
    '\t\tm_bLoading = false;\n\n'
    '\t\tGetGame().GetCallqueue().Remove(UpdateHudPositionPreview);\n'
    '\t\tGetGame().GetCallqueue().CallLater(UpdateHudPositionPreview, 0, false);\n'
    '\t\tGetGame().GetCallqueue().CallLater(UpdateHudPositionPreview, 250, true);\n'
    '\t}\n\n'
    '\t//------------------------------------------------------------------------------------------------\n'
    '\toverride void OnTabHide()\n'
    '\t{\n'
    '\t\tGetGame().GetCallqueue().Remove(UpdateHudPositionPreview);\n'
    '\t\tsuper.OnTabHide();\n'
    '\t}\n',
    1
)

marker = '\n\t//------------------------------------------------------------------------------------------------\n\tprotected void SetupHudControls()\n'
preview_code = r'''
	//------------------------------------------------------------------------------------------------
	protected void SetupHudPositionPreview()
	{
		m_PreviewHost = m_wRoot.FindAnyWidget("HUDPreviewHost");
		m_PreviewSquare = m_wRoot.FindAnyWidget("HUDPreviewSquare");
		m_PreviewSquareBackground = m_wRoot.FindAnyWidget("HUDPreviewSquareBackground");
		m_ScreenPreview = m_wRoot.FindAnyWidget("HUDScreenPreview");
		m_ScreenPreviewBackground = m_wRoot.FindAnyWidget("HUDScreenPreviewBackground");
		m_HudPositionPreview = m_wRoot.FindAnyWidget("HUDPositionPreview");
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateHudPositionPreview()
	{
		if (!m_PreviewHost || !m_PreviewSquare || !m_PreviewSquareBackground || !m_ScreenPreview || !m_ScreenPreviewBackground || !m_HudPositionPreview)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		float hostWidthPx;
		float hostHeightPx;
		m_PreviewHost.GetScreenSize(hostWidthPx, hostHeightPx);
		float hostWidth = workspace.DPIUnscale(hostWidthPx);
		float hostHeight = workspace.DPIUnscale(hostHeightPx);
		if (hostWidth <= 1 || hostHeight <= 1)
			return;

		// The outer preview stays square, while the inner screen preserves the player's
		// actual current display aspect ratio. This makes ultrawide, 16:9, 16:10 and
		// other resolutions preview the same normalized HUD placement used in game.
		float squareSize = Math.Min(hostWidth, hostHeight) - 24;
		if (squareSize <= 32)
			return;

		float squareLeft = (hostWidth - squareSize) * 0.5;
		float squareTop = (hostHeight - squareSize) * 0.5;
		FrameSlot.SetPos(m_PreviewSquare, squareLeft, squareTop);
		FrameSlot.SetSize(m_PreviewSquare, squareSize, squareSize);
		FrameSlot.SetPos(m_PreviewSquareBackground, 0, 0);
		FrameSlot.SetSize(m_PreviewSquareBackground, squareSize, squareSize);

		float screenWidth = workspace.GetWidth();
		float screenHeight = workspace.GetHeight();
		if (screenWidth <= 0 || screenHeight <= 0)
			return;

		float inset = 24;
		float available = squareSize - inset * 2;
		if (available <= 1)
			return;

		float screenAspect = screenWidth / screenHeight;
		float previewWidth = available;
		float previewHeight = available / screenAspect;
		if (previewHeight > available)
		{
			previewHeight = available;
			previewWidth = available * screenAspect;
		}

		float screenLeft = (squareSize - previewWidth) * 0.5;
		float screenTop = (squareSize - previewHeight) * 0.5;
		FrameSlot.SetPos(m_ScreenPreview, screenLeft, screenTop);
		FrameSlot.SetSize(m_ScreenPreview, previewWidth, previewHeight);
		FrameSlot.SetPos(m_ScreenPreviewBackground, 0, 0);
		FrameSlot.SetSize(m_ScreenPreviewBackground, previewWidth, previewHeight);

		HOTASDebugController controller = HOTASDebugController.GetInstance();
		int positionIndex = controller.GetSettingOptionIndex(1);
		float hudScale = 0.5 + controller.GetSettingOptionIndex(2) * 0.1;

		float hudWidth = 700 * hudScale;
		float hudHeight = 70 * hudScale;
		float marginX = 48 * hudScale;
		float marginY = 54 * hudScale;
		float hudLeft = (screenWidth - hudWidth) * 0.5;
		float hudTop = screenHeight - hudHeight - marginY;

		switch (positionIndex)
		{
			case 0: hudLeft = marginX; hudTop = marginY; break;
			case 1: hudLeft = (screenWidth - hudWidth) * 0.5; hudTop = marginY; break;
			case 2: hudLeft = screenWidth - hudWidth - marginX; hudTop = marginY; break;
			case 3: hudLeft = marginX; hudTop = (screenHeight - hudHeight) * 0.5; break;
			case 4: hudLeft = (screenWidth - hudWidth) * 0.5; hudTop = (screenHeight - hudHeight) * 0.5; break;
			case 5: hudLeft = screenWidth - hudWidth - marginX; hudTop = (screenHeight - hudHeight) * 0.5; break;
			case 6: hudLeft = marginX; hudTop = screenHeight - hudHeight - marginY; break;
			case 7: hudLeft = (screenWidth - hudWidth) * 0.5; hudTop = screenHeight - hudHeight - marginY; break;
			case 8: hudLeft = screenWidth - hudWidth - marginX; hudTop = screenHeight - hudHeight - marginY; break;
		}

		float previewHudWidth = previewWidth * (hudWidth / screenWidth);
		float previewHudHeight = previewHeight * (hudHeight / screenHeight);
		float previewHudLeft = previewWidth * (hudLeft / screenWidth);
		float previewHudTop = previewHeight * (hudTop / screenHeight);

		if (previewHudWidth < 8)
			previewHudWidth = 8;
		if (previewHudHeight < 5)
			previewHudHeight = 5;

		FrameSlot.SetPos(m_HudPositionPreview, previewHudLeft, previewHudTop);
		FrameSlot.SetSize(m_HudPositionPreview, previewHudWidth, previewHudHeight);

		if (controller.GetSettingOptionIndex(0) == 0)
			m_HudPositionPreview.SetOpacity(0.3);
		else
			m_HudPositionPreview.SetOpacity(0.9);
	}
'''
if marker not in src:
    raise SystemExit('SetupHudControls marker not found')
src = src.replace(marker, '\n' + preview_code + marker, 1)

old = ('\t\t\tHOTASDebugController.GetInstance().SetSettingOptionIndex(i, optionIndex);\n'
       '\t\t\treturn;\n')
new = ('\t\t\tHOTASDebugController.GetInstance().SetSettingOptionIndex(i, optionIndex);\n'
       '\t\t\tUpdateHudPositionPreview();\n'
       '\t\t\treturn;\n')
if old not in src:
    raise SystemExit('OnHudSettingChanged marker not found')
src = src.replace(old, new, 1)
script_path.write_text(src, encoding='utf-8')

layout = layout_path.read_text(encoding='utf-8')
ending = '  }\n }\n}\n'
if not layout.endswith(ending):
    raise SystemExit('Unexpected HOTASSettings.layout ending')

right_pane = r'''  }
  VerticalLayoutWidgetClass "{8C52D9F7A31B6500}" {
   Name "HUDPreviewPane"
   Slot LayoutSlot "{8C52D9F7A31B6501}" {
    Padding 36 72 24 24
    SizeMode Fill
    FillWeight 1
   }
   {
    VerticalLayoutWidgetClass "{8C52D9F7A31B6502}" : "{FEEEB639F2735BA1}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsTitle.layout" {
     Name "TitleHUDPreview"
     Slot LayoutSlot "{8C52D9F7A31B6503}" {
     }
     components {
      SCR_LabelComponent "{58B30C1A8E56F0FF}" {
       m_sLabel "HUD Position Preview"
       m_fPaddingTop 4
      }
     }
    }
    FrameWidgetClass "{8C52D9F7A31B6504}" {
     Name "HUDPreviewHost"
     Slot LayoutSlot "{8C52D9F7A31B6505}" {
      Padding 8 12 8 12
      SizeMode Fill
      FillWeight 1
     }
     {
      FrameWidgetClass "{8C52D9F7A31B6506}" {
       Name "HUDPreviewSquare"
       Slot FrameWidgetSlot "{8C52D9F7A31B6507}" {
        PositionX 0
        PositionY 0
        SizeX 400
        SizeY 400
        Alignment 0 0
       }
       {
        ImageWidgetClass "{8C52D9F7A31B6508}" {
         Name "HUDPreviewSquareBackground"
         Slot FrameWidgetSlot "{8C52D9F7A31B6509}" {
          PositionX 0
          PositionY 0
          SizeX 400
          SizeY 400
          Alignment 0 0
         }
         Color 0.08 0.08 0.08 0.72
         Size 1024 1024
        }
        FrameWidgetClass "{8C52D9F7A31B6510}" {
         Name "HUDScreenPreview"
         Slot FrameWidgetSlot "{8C52D9F7A31B6511}" {
          PositionX 20
          PositionY 80
          SizeX 360
          SizeY 240
          Alignment 0 0
         }
         {
          ImageWidgetClass "{8C52D9F7A31B6512}" {
           Name "HUDScreenPreviewBackground"
           Slot FrameWidgetSlot "{8C52D9F7A31B6513}" {
            PositionX 0
            PositionY 0
            SizeX 360
            SizeY 240
            Alignment 0 0
           }
           Color 0.012 0.016 0.021 1
           Size 1024 1024
          }
          ImageWidgetClass "{8C52D9F7A31B6514}" {
           Name "HUDPositionPreview"
           Slot FrameWidgetSlot "{8C52D9F7A31B6515}" {
            PositionX 120
            PositionY 205
            SizeX 120
            SizeY 16
            Alignment 0 0
           }
           Color 0.7605 0.3865 0.0802 1
           Size 1024 1024
          }
         }
        }
       }
      }
     }
    }
   }
  }
 }
}
'''
layout = layout[:-len(ending)] + right_pane
layout_path.write_text(layout, encoding='utf-8')
