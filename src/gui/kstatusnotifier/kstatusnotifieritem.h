/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSTATUSNOTIFIERITEM_H
#define KSTATUSNOTIFIERITEM_H

#include <QMenu>
#include <QObject>
#include <QPoint>
#include <QString>

//#include <kstatusnotifieritem_export.h>
#include "config-kstatusnotifieritem.h"

#include <memory>

class QAction;

class KStatusNotifierItemPrivate;

/*!
 * \class KStatusNotifierItem
 * \inmodule KStatusNotifierItem
 *
 * \brief KDE Status Notifier Item protocol implementation.
 *
 * This class implements the Status notifier Item D-Bus specification.
 * It provides an icon similar to the classical systemtray icons,
 * with some key differences:
 *
 * \list
 * \li the actual representation is done by the systemtray (or the app behaving
 *   like it) itself, not by this app.  Since 4.5 this also includes the menu,
 *   which means you cannot use embed widgets in the menu.
 *
 * \li there is communication between the systemtray and the icon owner, so the
 *   system tray can know if the application is in a normal or in a requesting
 *   attention state.
 *
 * \li icons are divided in categories, so the systemtray can represent in a
 *   different way the icons from normal applications and for instance the ones
 *   about hardware status.
 * \endlist
 *
 * Whenever possible you should prefer passing icon by name rather than by
 * pixmap because:
 *
 * \list
 * \li it is much lighter on D-Bus (no need to pass all image pixels).
 *
 * \li it makes it possible for the systemtray to load an icon of the appropriate
 *   size or to replace your icon with a systemtray specific icon which matches
 *   with the desktop theme.
 *
 * \li some implementations of the system tray do not support passing icons by
 *   pixmap and will show a blank icon instead.
 * \endlist
 *
 * \note When used inside a Flatpak it is important to request explicit support
 * in the Flatpak manifest with the following line:
 * \c --talk-name=org.kde.StatusNotifierWatcher
 *
 * \since 4.4
 */
class KSTATUSNOTIFIERITEM_EXPORT KStatusNotifierItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(ItemCategory category READ category WRITE setCategory)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(ItemStatus status READ status WRITE setStatus)
    Q_PROPERTY(QString iconName READ iconName WRITE setIconByName)
    Q_PROPERTY(QString overlayIconName READ overlayIconName WRITE setOverlayIconByName)
    Q_PROPERTY(QString attentionIconName READ attentionIconName WRITE setAttentionIconByName)
    Q_PROPERTY(QString toolTipIconName READ toolTipIconName WRITE setToolTipIconByName)
    Q_PROPERTY(QString toolTipTitle READ toolTipTitle WRITE setToolTipTitle)
    Q_PROPERTY(QString toolTipSubTitle READ toolTipSubTitle WRITE setToolTipSubTitle)

    friend class KStatusNotifierItemDBus;
    friend class KStatusNotifierItemPrivate;

public:
    /*!
     * All the possible status this icon can have, depending on the
     * importance of the events that happens in the parent application.
     *
     * \value Passive
     *        Nothing is happening in the application, so showing this icon is
     *        not required. This is the default value.
     *
     * \value Active
     *        The application is doing something, or it is important that the
     *        icon is always reachable from the user.
     *
     * \value NeedsAttention
     *        The application requests the attention of the user, for instance
     *        battery running out or a new IM message was received.
     */
    enum ItemStatus {
        Passive = 1,
        Active = 2,
        NeedsAttention = 3,
    };
    Q_ENUM(ItemStatus)

    /*!
     * Different kinds of applications announce their type to the systemtray,
     * so can be drawn in a different way or in a different place.
     *
     * \value ApplicationStatus
     *        An icon for a normal application, can be seen as its taskbar
     *        entry. This is the default value.
     *
     * \value Communications
     *        This is a communication oriented application; this icon will be used
     *        for things such as the notification of a new message.
     *
     * \value SystemServices
     *        This is a system service, it can show itself in the system tray if
     *        it requires interaction from the user or wants to inform him about
     *        something.
     *
     * \value Hardware
     *        This application shows hardware status or a means to control it.
     *
     * \value Reserved
     */
    enum ItemCategory {
        ApplicationStatus = 1,
        Communications = 2,
        SystemServices = 3,
        Hardware = 4,
        Reserved = 129,
    };
    Q_ENUM(ItemCategory)

    /*!
     * \brief Construct a new status notifier item.
     *
     * \a parent the parent object for this object. If the object passed in as
     * a parent is also a QWidget, it will  be used as the main application window
     * represented by this icon and will be shown/hidden when an activation is requested.
     *
     * \sa associatedWidget
     **/
    explicit KStatusNotifierItem(QObject *parent = nullptr);

    /*!
     * \brief Construct a new status notifier item with a unique identifier.
     *
     * If your application has more than one status notifier item and the user
     * should be able to manipulate them separately (e.g. mark them for hiding
     * in a user interface), the id can be used to differentiate between them.
     *
     * The id should remain consistent even between application restarts.
     * Status notifier items without ids default to the application's name for the id.
     * This id may be used, for instance, by hosts displaying status notifier items to
     * associate configuration information with this item in a way that can persist
     * between sessions or application restarts.
     *
     * \a id the unique id for this icon
     *
     * \a parent the parent object for this object. If the object passed in as
     * a parent is also a QWidget, it will  be used as the main application window
     * represented by this icon and will be shown/hidden when an activation is requested.
     *
     * \sa associatedWidget
     **/
    explicit KStatusNotifierItem(const QString &id, QObject *parent = nullptr);

    ~KStatusNotifierItem() override;

    /*!
     * \brief Returns the id which was specified in the constructor.
     *
     * This should be guaranteed to be consistent between application starts and
     * untranslated, as host applications displaying items may use it for
     * storing configuration related to this item.
     */
    QString id() const;

    /*!
     * \brief Sets the category for this icon.
     *
     * Usually it's needed to call this function only once.
     *
     * \a category the new category for this icon
     */
    void setCategory(const ItemCategory category);

    /*!
     * \brief Returns the application category.
     */
    ItemCategory category() const;

    /*!
     * \brief Sets a \a title for this icon.
     */
    void setTitle(const QString &title);

    /*!
     * \brief Returns the title of this icon.
     */
    QString title() const;

    /*!
     * \brief Sets a new \a status for this icon.
     */
    void setStatus(const ItemStatus status);

    /*!
     * \brief Returns the current application status.
     */
    ItemStatus status() const;

    // Main icon related functions
    /*!
     * \brief Sets a new main icon for the system tray.
     *
     * \a name it must be a QIcon::fromTheme compatible name, this is
     *         the preferred way to set an icon
     */
    void setIconByName(const QString &name);

    /*!
     * \brief Returns the name of the main icon to be displayed.
     *
     * If image() is not empty this will always return an empty string
     */
    QString iconName() const;

    /*!
     * \brief Sets a new main icon for the system tray.
     *
     * \a icon our icon, use setIcon(const QString) when possible
     */
    void setIconByPixmap(const QIcon &icon);

    /*!
     * \brief Returns a pixmap of the icon.
     */
    QIcon iconPixmap() const;

    /*!
     * \brief Sets an icon to be used as overlay for the main one.
     *
     * \a name the icon name, if name is and empty QString()
     *         (and overlayIconPixmap() is empty too) the icon will be removed
     */
    void setOverlayIconByName(const QString &name);

    /*!
     * \brief Returns the name of the icon to be used as overlay fr the main one.
     */
    QString overlayIconName() const;

    /*!
     * \brief Sets an icon to be used as overlay for the main one.
     *
     *   setOverlayIconByPixmap(QIcon()) will remove the overlay when
     *   overlayIconName() is empty too.
     *
     * \a icon our overlay icon, use setOverlayIcon(const QString) when possible.
     */
    void setOverlayIconByPixmap(const QIcon &icon);

    /*!
     * Returns a pixmap of the icon
     */
    QIcon overlayIconPixmap() const;

    // Requesting attention icon

    /*!
     * \brief Sets a new icon that should be used when the application
     * wants to request attention (usually the systemtray
     * will blink between this icon and the main one).
     *
     * \a name QIcon::fromTheme compatible name of icon to use
     */
    void setAttentionIconByName(const QString &name);

    /*!
     * \brief Returns the name of the icon to be displayed when the application
     * is requesting the user's attention.
     *
     * If attentionImage() is not empty this will always return an empty string.
     */
    QString attentionIconName() const;

    /*!
     * \brief Sets the pixmap of the requesting attention icon.
     *
     * Use setAttentionIcon(const QString) instead when possible.
     *
     * \a icon QIcon to use for requesting attention.
     */
    void setAttentionIconByPixmap(const QIcon &icon);

    /*!
     * Returns a pixmap of the requesting attention icon
     */
    QIcon attentionIconPixmap() const;

    /*!
     * \brief Sets a movie \a name as the requesting attention icon.
     *
     * This overrides anything set in setAttentionIcon()
     */
    void setAttentionMovieByName(const QString &name);

    /*!
     * \brief Returns the name of the movie to be displayed when the application is
     * requesting the user attention.
     */
    QString attentionMovieName() const;

    // ToolTip handling
    /*!
     * \brief Sets a new toolTip for this icon.
     *
     * A toolTip is composed of an icon,
     * a title and a text, all fields are optional.
     *
     * \a iconName a QIcon::fromTheme compatible name for the tootip icon
     *
     * \a title tootip title
     *
     * \a subTitle subtitle for the toolTip
     */
    void setToolTip(const QString &iconName, const QString &title, const QString &subTitle);

    /*!
     * \brief Sets a new toolTip or this status notifier item.
     *
     * \overload setTooltip()
     *
     * \a icon a QIcon() pixmap for the tooltip icon
     *
     * \a title tootip title
     *
     * \a subTitle subtitle for the toolTip
     */
    void setToolTip(const QIcon &icon, const QString &title, const QString &subTitle);

    /*!
     * \brief Sets a new icon for the toolTip.
     *
     * \a name the name for the icon
     */
    void setToolTipIconByName(const QString &name);

    /*!
     * \brief Returns the name of the toolTip icon.
     *
     * If toolTipImage() is not empty this will always return an empty string.
     */
    QString toolTipIconName() const;

    /*!
     * \brief Set a new icon for the toolTip.
     *
     * Use setToolTipIconByName(QString) if possible.
     *
     * \a icon representing the icon
     */
    void setToolTipIconByPixmap(const QIcon &icon);

    /*!
     * \brief Returns a serialization of the toolTip icon data.
     */
    QIcon toolTipIconPixmap() const;

    /*!
     * \brief Sets a new \a title for the toolTip.
     */
    void setToolTipTitle(const QString &title);

    /*!
     * \brief Returns the title of the main icon toolTip.
     */
    QString toolTipTitle() const;

    /*!
     * \brief Sets a new \a subTitle for the toolTip.
     */
    void setToolTipSubTitle(const QString &subTitle);

    /*!
     * \brief Returns the subtitle of the main icon toolTip.
     */
    QString toolTipSubTitle() const;

    /*!
     * \brief Sets a new context \a menu for this StatusNotifierItem.
     *
     * the menu will be shown with a contextMenu(int,int)
     * call by the systemtray over D-Bus
     *
     * The KStatusNotifierItem instance takes ownership of the menu,
     * and will delete it upon its destruction.
     */
    void setContextMenu(QMenu *menu);

    /*!
     * \brief Access the context menu associated to this status notifier item.
     */
    QMenu *contextMenu() const;

    /*!
     * \brief Sets the main widget associated with this StatusNotifierItem.
     *
     * \a window The window to be used.
     *
     * \since 6.0
     */
    void setAssociatedWidget(QWidget *widget);

    /*!
     * \brief Access the main widget associated with this StatusNotifierItem.
     *
     * \since 6.0
     */
    QWidget *associatedWidget() const;

#if KSTATUSNOTIFIERITEM_ENABLE_DEPRECATED_SINCE(6, 6)
    /*!
     * \brief All the actions present in the menu.
     *
     * \deprecated[6.6] Read actions using contextMenu() instead.
     */
    KSTATUSNOTIFIERITEM_DEPRECATED_VERSION(6, 6, "Read actions from contextMenu()")
    QList<QAction *> actionCollection() const;
#endif

#if KSTATUSNOTIFIERITEM_ENABLE_DEPRECATED_SINCE(6, 6)
    /*!
     * \brief Adds an action to the actionCollection().
     *
     * \a name the name of the action
     *
     * \a action the action we want to add
     *
     * \deprecated[6.6] Add actions using contextMenu() instead.
     */
    KSTATUSNOTIFIERITEM_DEPRECATED_VERSION(6, 6, "Add actions to contextMenu()")
    void addAction(const QString &name, QAction *action);
#endif

#if KSTATUSNOTIFIERITEM_ENABLE_DEPRECATED_SINCE(6, 6)
    /*!
     * \brief Removes an action from the collection.
     *
     * \a name the name of the action
     *
     * \deprecated [6.6] Remove actions using contextMenu() instead.
     */
    KSTATUSNOTIFIERITEM_DEPRECATED_VERSION(6, 6, "Remove actions from contextMenu()")
    void removeAction(const QString &name);
#endif

#if KSTATUSNOTIFIERITEM_ENABLE_DEPRECATED_SINCE(6, 6)
    /*!
     * \brief Retrieves an action from the action collection by the action name.
     *
     * \a name the name of the action to retrieve
     *
     * \since 5.12
     *
     * \deprecated [6.6] Read actions using contextMenu(). For controlling
     *                   the behavior of the Quit action use quitRequested()
     *                   and abortQuit()
     */
    KSTATUSNOTIFIERITEM_DEPRECATED_VERSION(6, 6, "See API docs")
    QAction *action(const QString &name) const;
#endif

    /*!
     * \a enabled Whether to show the standard items in the menu, such as Quit.
     */
    void setStandardActionsEnabled(bool enabled);

    /*!
     * \brief Returns if the standard items in the menu, such as Quit.
     */
    bool standardActionsEnabled() const;

    /*!
     * \brief Shows the user a notification.
     *
     * If possible use KNotify instead.
     *
     * \a title message title
     *
     * \a message the actual text shown to the user
     *
     * \a icon icon to be shown to the user
     *
     * \a timeout how much time will elapse before hiding the message
     */
    void showMessage(const QString &title, const QString &message, const QString &icon, int timeout = 10000);

    /*!
     * \brief Returns the last provided token to be used with Wayland's xdg_activation_v1.
     */
    QString providedToken() const;

    /*!
     * \brief Cancelles an ongoing quit operation.
     *
     * Call this in a slot connected to quitRequested().
     *
     * \sa quitRequested()
     *
     * \since 6.5
     */
    void abortQuit();

    /*!
     * Indictates that this item only supports the context menu. Instead of sending
     * activate the provided the menu will be shown.
     *
     * \sa setContextMenu
     *
     * \since 6.14
     */
    void setIsMenu(bool isMenu);

    /*!
     * Returns if the item indicates that it only supports the context menu.
     *
     * \since 6.14
     */
    bool isMenu() const;

public Q_SLOTS:

    /*!
     * \brief Shows the main window and try to position it on top
     * of the other windows, if the window is already visible, hide it.
     *
     * \a pos if it's a valid position it represents the mouse coordinates when the event was triggered
     */
    virtual void activate(const QPoint &pos = QPoint());

    /*!
     * \brief Hides the main window, if not already hidden.
     *
     * Stores some information about the window which otherwise would be lost due to unmapping
     * from the window system. Use when toggling the main window via activate(const QPoint &)
     * is not wanted, but instead the hidden state should be reached in any case.
     *
     * \since 6.0
     */
    void hideAssociatedWidget();

Q_SIGNALS:
    /*!
     * \brief Inform the host application that the mouse wheel
     * (or another mean of scrolling that the visualization provides) has been used.
     *
     * \a delta the amount of scrolling, can be either positive or negative
     *
     * \a orientation direction of the scrolling, can be either horizontal or vertical
     */
    void scrollRequested(int delta, Qt::Orientation orientation);

    /*!
     * \brief Inform the host application that an activation has been requested.
     *
     *           For instance left mouse click, but this is not guaranteed since
     *           it's dependent from the visualization
     *
     * \a active if it's true the application asked for the activation
     *              of the main window, if it's false it asked for hiding
     *
     * \a pos the position in the screen where the user clicked to
     *  trigger this signal, QPoint() if it's not the consequence of a mouse click.
     */
    void activateRequested(bool active, const QPoint &pos);

    /*!
     * \brief Alternate activate action.
     *
     * For instance right mouse click, but this is not guaranteed since
     * it's dependent from the visualization
     *
     * \a pos the position in the screen where the user clicked to
     *  trigger this signal, QPoint() if it's not the consequence of a mouse click.
     */
    void secondaryActivateRequested(const QPoint &pos);

    /*!
     * \brief Emitted when the Quit action is triggered.
     *
     * If abortQuit() is called from the slot the quit is cancelled.
     * This allows to e.g. display a custom confirmation prompt.
     *
     * \since 6.5
     */
    void quitRequested();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    std::unique_ptr<KStatusNotifierItemPrivate> const d;

    Q_PRIVATE_SLOT(d, void serviceChange(const QString &name, const QString &oldOwner, const QString &newOwner))
    Q_PRIVATE_SLOT(d, void contextMenuAboutToShow())
    Q_PRIVATE_SLOT(d, void quit())
    Q_PRIVATE_SLOT(d, void minimizeRestore())
    Q_PRIVATE_SLOT(d, void legacyWheelEvent(int))
    Q_PRIVATE_SLOT(d, void legacyActivated(QSystemTrayIcon::ActivationReason))
};

#endif
