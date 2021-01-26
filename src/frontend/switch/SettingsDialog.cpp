#include "SettingsDialog.h"

#include "Style.h"
#include "KeyExplanations.h"
#include "BackButton.h"
#include "main.h"

#include "PlatformConfig.h"

#include <string.h>

namespace SettingsDialog
{
const char* SettingsPrefix = "settingsdialog_entries";

const char* ComboboxElementPrefix = "settings_combobox";

void DoSlider(BoxGui::Frame& parent, BoxGui::Skewer& skewer, const char* name, int& value, int low, int high, bool first = false)
{
    BoxGui::Frame settingFrame{parent, skewer.Spit({parent.Area.Size.X, UIRowHeight}, Gfx::align_Right),
        {5.f, 5.f}, {5.f, 5.f}};

    bool selected = BoxGui::InputElement(settingFrame, BoxGui::MakeUniqueName(SettingsPrefix, BoxGui::MakeUniqueName(name, 0)));
    if (selected && BoxGui::LeftPressed())
    {
        value--;
        if (value < low)
            value = low;
    }
    if (selected && BoxGui::RightPressed())
    {
        value++;
        if (value > high)
            value = high;
    }

    // a bit wasteful
    Gfx::DrawRectangle(settingFrame.Area.Position - Gfx::Vector2f{0.f, 5.f}, settingFrame.Area.Size + Gfx::Vector2f{0.f, 5.f*2.f}, WidgetColorBright, true);

    if (selected)
        Gfx::DrawRectangle(settingFrame.Area.Position, settingFrame.Area.Size, WidgetColorVibrant);

    BoxGui::Skewer settingSkewer{settingFrame, settingFrame.Area.Size.Y/2.f, BoxGui::direction_Horizontal};

    settingSkewer.AlignLeft(15.f);
    BoxGui::Frame nameFrame{settingFrame, settingSkewer.Spit({settingFrame.Area.Size.X*0.8f, TextLineHeight})};
    Gfx::DrawText(Gfx::SystemFontStandard, nameFrame.Area.Position, TextLineHeight, DarkColor, "%s", name);

    settingSkewer.AlignRight(20.f);
    BoxGui::Frame valueFrame{settingFrame, settingSkewer.Spit({30.f, TextLineHeight})};
    Gfx::DrawText(Gfx::SystemFontStandard, valueFrame.Area.Position, TextLineHeight, DarkColor, "%d", value);

    settingSkewer.Advance(15.f);

    BoxGui::Frame bar{settingFrame, settingSkewer.Spit({150.f, 5.f})};
    Gfx::DrawRectangle(bar.Area.Position, bar.Area.Size, DarkColor);

    Gfx::DrawRectangle(bar.Area.Position + Gfx::Vector2f({(float)(value - low) / (high - low + 1) * bar.Area.Size.X, bar.Area.Size.Y/2.f-TextLineHeight/2.f}), {10.f, TextLineHeight}, DarkColor);

    if (!first)
    {
        Gfx::DrawRectangle(settingFrame.Area.Position + Gfx::Vector2f{10.f, -(5.f + 1.f)},
            {settingFrame.Area.Size.X - 2*10.f, 2.f},
            SeparatorColor);
    }
}

void DoCheckbox(BoxGui::Frame& parent, BoxGui::Skewer& skewer, const char* name, bool& value, bool first = false)
{
    BoxGui::Frame settingFrame{parent, skewer.Spit({parent.Area.Size.X, UIRowHeight}, Gfx::align_Right),
        {5.f, 5.f}, {5.f, 5.f}};

    bool selected = BoxGui::InputElement(settingFrame, BoxGui::MakeUniqueName(SettingsPrefix, BoxGui::MakeUniqueName(name, 0)));
    if (selected && BoxGui::ConfirmPressed())
    {
        value ^= true;
    }
    if (selected)
    {
        KeyExplanation::Explain(KeyExplanation::button_A, "Toggle");
    }

    // a bit wasteful
    Gfx::DrawRectangle(settingFrame.Area.Position - Gfx::Vector2f{0.f, 5.f}, settingFrame.Area.Size + Gfx::Vector2f{0.f, 5.f*2.f}, WidgetColorBright, true);

    if (selected)
        Gfx::DrawRectangle(settingFrame.Area.Position, settingFrame.Area.Size, WidgetColorVibrant);

    BoxGui::Skewer settingSkewer{settingFrame, settingFrame.Area.Size.Y/2.f, BoxGui::direction_Horizontal};

    settingSkewer.AlignLeft(15.f);
    BoxGui::Frame nameFrame{settingFrame, settingSkewer.Spit({settingFrame.Area.Size.X*0.8f, TextLineHeight})};
    Gfx::DrawText(Gfx::SystemFontStandard, nameFrame.Area.Position, TextLineHeight, DarkColor, "%s", name);

    settingSkewer.AlignRight(25.f);
    BoxGui::Frame checkMarkFrame{settingFrame, settingSkewer.Spit({TextLineHeight, TextLineHeight})};

    Gfx::DrawText(Gfx::SystemFontNintendoExt,
        checkMarkFrame.Area.Position, TextLineHeight,
        DarkColor,
        value ? GFX_NINTENDOFONT_CHECKMARK : GFX_NINTENDOFONT_CROSS);

    if (!first)
    {
        Gfx::DrawRectangle(settingFrame.Area.Position + Gfx::Vector2f{10.f, -(5.f + 1.f)},
            {settingFrame.Area.Size.X - 2*10.f, 2.f},
            SeparatorColor);
    }
}

void DoCombobox(BoxGui::Frame& parent, BoxGui::Skewer& skewer, const char* name, const char* options, int& selectedOption, bool first = false)
{
    BoxGui::Frame settingFrame{parent, skewer.Spit({parent.Area.Size.X, UIRowHeight}, Gfx::align_Right),
        {5.f, 5.f}, {5.f, 5.f}};

    bool selected = BoxGui::InputElement(settingFrame, BoxGui::MakeUniqueName(SettingsPrefix, BoxGui::MakeUniqueName(name, 0)));
    if (selected && BoxGui::ConfirmPressed())
    {
        struct Dialog
        {
            int OriginalValue;
            int& SelectedOption;
            const char* Name, *Options;
            double StartTimestamp;
            double EndTimestamp = -INFINITY;

            bool operator()(BoxGui::Frame& rootFrame)
            {
                Gfx::Color color = DarkColor;
                // fade in
                color.A = (float)std::min((Gfx::AnimationTimestamp - StartTimestamp) * 5.0, 0.8);
                Gfx::DrawRectangle(rootFrame.Area.Position, rootFrame.Area.Size, color);

                Gfx::Vector2f Size = rootFrame.Area.Size * 0.9f;
                Size.X = std::min(Size.X, 720.f);
                BoxGui::Frame dialogFrame{rootFrame, rootFrame.Area.CenteredChild(Size)};

                Gfx::DrawRectangle(dialogFrame.Area.Position, dialogFrame.Area.Size, WidgetColorBright, true);

                BoxGui::Skewer optionsSkewer{dialogFrame, 0.f, BoxGui::direction_Vertical};
                optionsSkewer.AlignLeft(30.f);
                BoxGui::Frame titleFrame{dialogFrame, optionsSkewer.Spit({dialogFrame.Area.Size.X, TextLineHeight * 2.f}, Gfx::align_Right), {5.f, 5.f}, {5.f, 5.f}};
                Gfx::DrawText(Gfx::SystemFontStandard, titleFrame.Area.Position + Gfx::Vector2f{15.f, 0.f}, TextLineHeight * 2.f, DarkColor,
                    Gfx::align_Left, Gfx::align_Left, Name);
                optionsSkewer.Advance(10.f);

                const char* curOption = Options;
                int i = 0;
                while (true)
                {
                    BoxGui::Frame optionFrame{dialogFrame,
                        optionsSkewer.Spit({dialogFrame.Area.Size.X, UIRowHeight}, Gfx::align_Right), {5.f, 5.f}, {5.f, 5.f}};

                    bool selected = BoxGui::InputElement(optionFrame, BoxGui::MakeUniqueName(ComboboxElementPrefix, i));
                    
                    if (selected && BoxGui::ConfirmPressed() && EndTimestamp < 0.0)
                    {
                        SelectedOption = i;
                        if (SelectedOption != OriginalValue)
                            EndTimestamp = Gfx::AnimationTimestamp;
                        else
                            EndTimestamp = 0.f;
                    } 
                    if (selected)
                        Gfx::DrawRectangle(optionFrame.Area.Position, optionFrame.Area.Size, WidgetColorVibrant);

                    BoxGui::Skewer optionSkewer{optionFrame, optionFrame.Area.Size.Y/2.f, BoxGui::direction_Horizontal};
                    if (SelectedOption == i)
                    {
                        optionSkewer.AlignLeft(20.f);
                        Gfx::DrawText(Gfx::SystemFontNintendoExt, optionSkewer.CurrentPosition(), TextLineHeight, DarkColor,
                            Gfx::align_Left, Gfx::align_Center, GFX_NINTENDOFONT_CHECKMARK);

                        KeyExplanation::Explain(KeyExplanation::button_A, "Choose");
                    }
                    optionSkewer.AlignLeft(50.f);

                    Gfx::DrawText(Gfx::SystemFontStandard, optionSkewer.CurrentPosition(), TextLineHeight, DarkColor,
                        Gfx::align_Left, Gfx::align_Center, curOption);

                    if (i > 0)
                    {
                        Gfx::DrawRectangle(optionFrame.Area.Position + Gfx::Vector2f{10.f, -(5.f + 1.f)},
                            {optionFrame.Area.Size.X - 2*10.f, 2.f},
                            SeparatorColor);
                    }

                    i++;
                    curOption += strlen(curOption) + 1; // skip the \0
                    if (*curOption == '\0')
                        break;
                }

                if (SelectedOption >= i)
                    SelectedOption = 0;

                KeyExplanation::Explain(KeyExplanation::button_B, "Cancel");

                const double fadeoutLength = 0.25;
                if (BoxGui::CancelPressed())
                {
                    EndTimestamp = 0.f;
                }

                KeyExplanation::DoGui(rootFrame);

                // don't close the dialog immediately
                // instead wait a moment so the user can reflect on their choice :D
                return EndTimestamp < 0.0 || Gfx::AnimationTimestamp - EndTimestamp < fadeoutLength;
            }
        };

        BoxGui::OpenModalDialog(Dialog{selectedOption, selectedOption, name, options, Gfx::AnimationTimestamp});
        BoxGui::ForceSelecton(BoxGui::MakeUniqueName(ComboboxElementPrefix, selectedOption), true, 1);
    }
    if (selected)
    {
        KeyExplanation::Explain(KeyExplanation::button_A, "Choose");
    }

    Gfx::DrawRectangle(settingFrame.Area.Position - Gfx::Vector2f{0.f, 5.f}, settingFrame.Area.Size + Gfx::Vector2f{0.f, 5.f*2.f}, WidgetColorBright, true);
    if (selected)
        Gfx::DrawRectangle(settingFrame.Area.Position, settingFrame.Area.Size, WidgetColorVibrant);

    BoxGui::Skewer settingSkewer{settingFrame, settingFrame.Area.Size.Y/2.f, BoxGui::direction_Horizontal};

    settingSkewer.AlignLeft(20.f);
    Gfx::DrawText(Gfx::SystemFontStandard, settingSkewer.CurrentPosition(), TextLineHeight, DarkColor,
        Gfx::align_Left, Gfx::align_Center, name);

    const char* selectedOptionName = options;
    for (u32 i = 0; i < selectedOption; i++)
    {
        selectedOptionName += strlen(selectedOptionName) + 1;
        if (*selectedOptionName == '\0')
        {
            selectedOptionName = "Error!!!";
            break;
        }
    }
    settingSkewer.AlignRight(20.f);
    Gfx::DrawText(Gfx::SystemFontStandard, settingSkewer.CurrentPosition(), TextLineHeight, DarkColor,
        Gfx::align_Right, Gfx::align_Center, selectedOptionName);

    if (!first)
    {
        Gfx::DrawRectangle(settingFrame.Area.Position + Gfx::Vector2f{10.f, -(5.f + 1.f)},
            {settingFrame.Area.Size.X - 2*10.f, 2.f},
            SeparatorColor);
    }
}

void SectionHeader(BoxGui::Frame& parent, BoxGui::Skewer& skewer, const char* name)
{
    const float height = TextLineHeight * 2.f;
    skewer.Advance(height);
    BoxGui::Frame nameFrame{parent, skewer.Spit({parent.Area.Size.X * 0.6f, height}, Gfx::align_Right), {5.f, 0.f}, {5.f, 15.f}};

    Gfx::DrawRectangle(nameFrame.Area.Position - Gfx::Vector2f{0.f, 15.f}, nameFrame.Area.Size + Gfx::Vector2f{0.f, 15.f*2.f}, WidgetColorBright, true);
    Gfx::DrawText(Gfx::SystemFontStandard, nameFrame.Area.Position + Gfx::Vector2f{10.f, 0.f}, height, DarkColor, "%s", name);
}

void DoGui(BoxGui::Frame& parent)
{
    BoxGui::Frame settingsFrame{parent,
        {{0.f, BackButtonHeight}, {parent.Area.Size.X, parent.Area.Size.Y - BackButtonHeight}},
        {0.f, 0.f}, {0.f, 0.f},
        BoxGui::direction_Vertical, BoxGui::MakeUniqueName(SettingsPrefix, -1), false, true};

    BoxGui::Skewer settingsSkewer{settingsFrame, 0.f, BoxGui::direction_Vertical};

    Gfx::PushScissor(settingsFrame.Area.Position.X, settingsFrame.Area.Position.Y, settingsFrame.Area.Size.X, settingsFrame.Area.Size.Y);

    const char* title = "error";
    switch (CurrentUiScreen)
    {
    case uiScreen_EmulationSettings:
        title = "Emulation settings";
        {
            SectionHeader(settingsFrame, settingsSkewer, "General");
            DoCombobox(settingsFrame, settingsSkewer, "Console mode", "DS\0DSi (experimental)\0", Config::ConsoleType, true);
            if (Config::ConsoleType == 0)
            {
                bool bootDirectly = Config::DirectBoot;
                DoCheckbox(settingsFrame, settingsSkewer, "Boot directly (Skip bios)", bootDirectly);
                Config::DirectBoot = bootDirectly;
            }
        }
        {
            bool jitEnable = Config::JIT_Enable;
            SectionHeader(settingsFrame, settingsSkewer, "JIT recompiler");
            bool branchOptimisations = Config::JIT_BranchOptimisations;
            bool literalOptimisations = Config::JIT_LiteralOptimisations;
            bool fastMemory = Config::JIT_FastMemory;

            DoCheckbox(settingsFrame, settingsSkewer, "Enable JIT reompiler", jitEnable, true);
            if (jitEnable)
            {
                DoSlider(settingsFrame, settingsSkewer, "Maximum block size", Config::JIT_MaxBlockSize, 1, 32);
                DoCheckbox(settingsFrame, settingsSkewer, "Enable JIT Branch Optimisations", branchOptimisations);
                DoCheckbox(settingsFrame, settingsSkewer, "Enable JIT Literal Optimisations", literalOptimisations);
                DoCheckbox(settingsFrame, settingsSkewer, "Enable JIT Fast Memory", fastMemory);
            }

            Config::JIT_Enable = jitEnable;
            Config::JIT_BranchOptimisations = branchOptimisations;
            Config::JIT_LiteralOptimisations = literalOptimisations;
            Config::JIT_FastMemory = fastMemory;
        }
        break;
    case uiScreen_DisplaySettings:
        title = "Presentation settings";
        {
            SectionHeader(settingsFrame, settingsSkewer, "Framerate");
            bool limitFramerate = Config::LimitFramerate;
            DoCheckbox(settingsFrame, settingsSkewer, "Limit framerate", limitFramerate, true);
            Config::LimitFramerate = limitFramerate;
        }
        {
            SectionHeader(settingsFrame, settingsSkewer, "GUI");

            DoCombobox(settingsFrame, settingsSkewer, "Global rotation", "0°\090°\000180°\000270°\0", Config::GlobalRotation, true);
            bool showPerformanceMetrics = Config::ShowPerformanceMetrics;
            DoCheckbox(settingsFrame, settingsSkewer, "Show performance metrics", showPerformanceMetrics);
            Config::ShowPerformanceMetrics = showPerformanceMetrics;
        }
        {
            SectionHeader(settingsFrame, settingsSkewer, "Screens");

            DoCombobox(settingsFrame, settingsSkewer, "Rotation", "0°\090°\000180°\000270°\0", Config::ScreenRotation, true);
            DoCombobox(settingsFrame, settingsSkewer, "Sizing", "Even\0Emphasise top\0Emphasise bottom\0Auto\0Top only\0Bottom only\0", Config::ScreenSizing);
            DoCombobox(settingsFrame, settingsSkewer, "Gap", "0px\0001px\08px\00064px\090px\000128px\0", Config::ScreenGap);
            DoCombobox(settingsFrame, settingsSkewer, "Layout", "Natural\0Vertical\0Horizontal\0Hybrid\0", Config::ScreenLayout);
            DoCombobox(settingsFrame, settingsSkewer, "Aspect ratio top", "4:3 (native)\00016:9\0", Config::ScreenAspectTop);
            DoCombobox(settingsFrame, settingsSkewer, "Aspect ratio bottom", "4:3 (native)\00016:9\0", Config::ScreenAspectBot);
            bool screenSwap = Config::ScreenSwap;
            DoCheckbox(settingsFrame, settingsSkewer, "Swap screens", screenSwap);
            Config::ScreenSwap = screenSwap;
            bool integerScaling = Config::IntegerScaling;
            DoCheckbox(settingsFrame, settingsSkewer, "Integer scaling", integerScaling);
            Config::IntegerScaling = integerScaling;
            DoCombobox(settingsFrame, settingsSkewer, "Filtering", "Nearest\0Nearest smooth edges\0Linear\0", Config::Filtering);
        }
        Emulation::UpdateScreenLayout();
        break;
    case uiScreen_InputSettings:
        title = "Input settings";
        {
            SectionHeader(settingsFrame, settingsSkewer, "Touchscreen");
            
            DoCombobox(settingsFrame, settingsSkewer, "Cursor mode", "Mouse mode\0Offset mode\0Motion controls!\0", Config::TouchscreenMode, true);
            DoCombobox(settingsFrame, settingsSkewer, "Click mode", "Hold\0Toggle\0", Config::TouchscreenClickMode);
            bool leftHanded = Config::LeftHandedMode;
            DoCheckbox(settingsFrame, settingsSkewer, "Left handed mode", leftHanded);
            Config::LeftHandedMode = leftHanded;
        }
        break;
    }
    Gfx::PopScissor();

    BackButton::DoGui(parent, title);

    KeyExplanation::Explain(KeyExplanation::button_B, "Back");
    if (BoxGui::CancelPressed())
        BackButton::GoBack();
}

}
