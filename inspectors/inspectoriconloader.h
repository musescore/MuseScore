#ifndef INSPECTORICONLOADER_H
#define INSPECTORICONLOADER_H


class InspectorIconLoader
{
public:
    static QIcon* icon(const char* iconName);

    static constexpr const char* TEXT_JUSTIFY_LEFT_ICON = "format-justify-left.svg";
    static constexpr const char* TEXT_JUSTIFY_CENTER_ICON = "format-justify-center.svg";
    static constexpr const char* TEXT_JUSTIFY_RIGHT_ICON = "format-justify-right.svg";

    static constexpr const char* TEXT_ALIGN_V_CENTER_ICON = "align-vertical-center.svg";
    static constexpr const char* TEXT_ALIGN_V_TOP_ICON = "align-vertical-top.svg";
    static constexpr const char* TEXT_ALIGN_V_BASELINE_ICON = "align-vertical-baseline.svg";
    static constexpr const char* TEXT_ALIGN_V_BOTTOM_ICON = "align-vertical-bottom.svg";

    static constexpr const char* TEXT_FORMAT_BOLD_ICON = "format-text-bold.svg";
    static constexpr const char* TEXT_FORMAT_ITALIC_ICON = "format-text-italic.svg";
    static constexpr const char* TEXT_FORMAT_UNDERLINE_ICON = "format-text-underline.svg";

    static constexpr const char* RESET_ICON = "edit-reset.svg";
private:
    InspectorIconLoader() = default;

    static QMap<QString, QIcon*> m_iconCache;
};

#endif // INSPECTORICONLOADER_H
