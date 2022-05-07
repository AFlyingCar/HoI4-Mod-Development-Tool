#ifndef STOCKICONS_H
# define STOCKICONS_H

# include <string_view>

/**
 * @brief Every stock-ID from gtkmm/stock.h converted to the new icon-name
 *        format.
 */
namespace HMDT::GUI::StockIcons {
    constexpr std::string_view DIALOG_AUTHENTICATION = "dialog-password";

    constexpr std::string_view DIALOG_INFO = "dialog-information";
    constexpr std::string_view DIALOG_WARNING = "dialog-warning";
    constexpr std::string_view DIALOG_ERRO = "dialog-error";
    constexpr std::string_view DIALOG_QUESTIO = "dialog-question";

    // constexpr std::string_view DND; // No replacement available.
    // constexpr std::string_view DND_MULTIPLE; // No replacement available.

    constexpr std::string_view ABOUT = "help-about"; // or the label "_About"
    constexpr std::string_view ADD = "list-add"; // or the label "_Add"
    // constexpr std::string_view APPLY; // Do not use an icon. Use label "_Apply"
    constexpr std::string_view BOLD = "format-text-bold";
    // constexpr std::string_view CANCEL; // Do not use an icon. Use label "_Cancel"
    constexpr std::string_view CAPS_LOCK_WARNING = "dialog-warning-symbolic";
    constexpr std::string_view CDROM = "media-optical";
    constexpr std::string_view CLEAR = "edit-clear";
    constexpr std::string_view CLOSE = "window-close"; // or the label "_Close"
    // constexpr std::string_view COLOR_PICKER; // No replacement available.
    // constexpr std::string_view CONVERT; // No replacement available.
    // constexpr std::string_view CONNECT; // No replacement available.
    constexpr std::string_view COPY = "edit-copy";
    constexpr std::string_view CUT = "edit-cut"; // or the label "Cu_t"
    constexpr std::string_view DELETE = "edit-delete"; // or the  label "_Delete"
    constexpr std::string_view DIRECTORY = "folder";
    // constexpr std::string_view DISCARD; // No replacement available.
    // constexpr std::string_view DISCONNECT; // No replacement available.
    // constexpr std::string_view EDIT; // No replacement available.
    constexpr std::string_view EXECUTE = "system-run";
    constexpr std::string_view FILE = "text-x-generic";
    constexpr std::string_view FIND = "edit-find";
    constexpr std::string_view FIND_AND_REPLACE = "edit-find-replace";
    // constexpr std::string_view FLOPPY; // No replacement available.
    constexpr std::string_view FULLSCREEN = "view-fullscreen";
    constexpr std::string_view LEAVE_FULLSCREEN = "view-restore";
    constexpr std::string_view GOTO_BOTTOM = "go-bottom";
    constexpr std::string_view GOTO_FIRST = "go-first";
    constexpr std::string_view GOTO_LAST = "go-last";
    constexpr std::string_view GOTO_TOP = "go-top";
    constexpr std::string_view GO_BACK = "go-previous";
    constexpr std::string_view GO_DOWN = "go-down";
    constexpr std::string_view GO_FORWARD = "go-next";
    constexpr std::string_view GO_UP = "go-up";
    constexpr std::string_view HARDDISK = "drive-harddisk";
    constexpr std::string_view HELP = "help-browser";
    constexpr std::string_view HOME = "go-home";
    // constexpr std::string_view INDEX; // No replacement available;
    constexpr std::string_view INFO = "dialog-information";
    constexpr std::string_view INDENT = "format-indent-more";
    constexpr std::string_view UNINDENT = "format-indent-less";
    constexpr std::string_view ITALIC = "format-text-italic";
    constexpr std::string_view JUMP_TO = "go-jump";
    constexpr std::string_view JUSTIFY_CENTER = "format-justify-center";
    constexpr std::string_view JUSTIFY_FILL = "format-justify-fill";
    constexpr std::string_view JUSTIFY_LEFT = "format-justify-left";
    constexpr std::string_view JUSTIFY_RIGHT = "format-justify-right";
    constexpr std::string_view MISSING_IMAGE = "image-missing";
    constexpr std::string_view MEDIA_FORWARD = "media-seek-forward"; // or the label "_Forward";
    constexpr std::string_view MEDIA_NEXT = "media-skip-forward"; // or the label "_Next";
    constexpr std::string_view MEDIA_PAUSE = "media-playback-pause"; // or the label "P_ause"
    constexpr std::string_view MEDIA_PLAY = "media-playback-start"; // or the label "_Play"
    constexpr std::string_view MEDIA_PREVIOUS = "media-skip-backward"; // or the label "Pre_vious"
    constexpr std::string_view MEDIA_RECORD = "media-record"; // or the label "_Record".
    constexpr std::string_view MEDIA_REWIND = "media-seek-backward"; // or the label "R_ewind"
    constexpr std::string_view MEDIA_STOP = "media-playback-stop"; // or the label "_Stop"
    constexpr std::string_view NETWORK = "network-workgroup";
    constexpr std::string_view NEW = "document-new"; // or the label "_New".
    // constexpr std::string_view NO; // No replacement available
    // constexpr std::string_view OK; // Do not use an icon. Use label "_OK"
    constexpr std::string_view OPEN = "document-open"; // or the label "_Open"
    // constexpr std::string_view ORIENTATION_PORTRAIT; // No replacement available
    constexpr std::string_view ORIENTATION_LANDSCAPE; // No replacement available
    constexpr std::string_view ORIENTATION_REVERSE_LANDSCAPE; // No replacement available
    constexpr std::string_view ORIENTATION_REVERSE_PORTRAIT; // No replacement available
    constexpr std::string_view PASTE = "edit-paste"; // or the label "_Paste"
    constexpr std::string_view PREFERENCES = "preferences-system"; // or the label "_Preferences"
    constexpr std::string_view PAGE_SETUP = "document-page-setup"; // or the label "Page Set_up"
    constexpr std::string_view PRINT = "document-print"; // or the label "_Print"
    constexpr std::string_view PRINT_ERROR = "printer-error";
    constexpr std::string_view PRINT_PREVIEW; // Use label "Pre_view"
    constexpr std::string_view PRINT_REPORT; // No replacement available
    constexpr std::string_view PRINT_WARNING; // No replacement available
    constexpr std::string_view PROPERTIES = "document-properties"; // or the label "_Properties"
    constexpr std::string_view QUIT = "application-exit"; // or the label "_Quit"
    constexpr std::string_view REDO = "edit-redo"; // or the label "_Redo"
    constexpr std::string_view REFRESH = "view-refresh"; // or the label "_Refresh"
    constexpr std::string_view REMOVE = "list-remove"; // or the label "_Remove"
    constexpr std::string_view REVERT_TO_SAVED = "document-revert"; // or the label "_Revert"
    constexpr std::string_view SAVE = "document-save"; // or the label "_Save"
    constexpr std::string_view SAVE_AS = "document-save-as"; // or the label "Save _As"
    constexpr std::string_view SELECT_ALL = "edit-select-all"; // or the label "Select _All"
    // constexpr std::string_view SELECT_COLOR; // No replacement available
    // constexpr std::string_view SELECT_FONT; // No replacement available
    constexpr std::string_view SORT_ASCENDING = "view-sort-ascending";
    constexpr std::string_view SORT_DESCENDING = "view-sort-descending";
    constexpr std::string_view SPELL_CHECK = "tools-check-spelling";
    constexpr std::string_view STOP = "process-stop"; // or the label "_Stop"
    constexpr std::string_view STRIKETHROUGH = "format-text-strikethrough"; // or the label "_Strikethrough"
    constexpr std::string_view UNDELETE; // No replacement available
    constexpr std::string_view UNDERLINE = "format-text-underline"; // or the label "_Underline"
    constexpr std::string_view UNDO = "edit-undo"; // or the label "_Undo"
    // constexpr std::string_view YES; // No replacement available
    constexpr std::string_view ZOOM_100 = "zoom-original"; // or the label "_Normal Size"
    constexpr std::string_view ZOOM_FIT = "zoom-fit-best"; // or the label "Best _Fit"
    constexpr std::string_view ZOOM_IN = "zoom-in"; // or the label "Zoom _In"
    constexpr std::string_view ZOOM_OUT = "zoom-out"; // or the label "Zoom _Out"
}

#endif

