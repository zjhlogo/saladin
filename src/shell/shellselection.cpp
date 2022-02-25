/**************************************************************************
 * This file is part of the Saladin program
 * Copyright (C) 2011-2017 Michał Męciński
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "shellselection.h"
#include "shelldropdata.h"
#include "shelldropdata_p.h"
#include "shellfolder.h"
#include "shellfolder_p.h"
#include "shellitem.h"
#include "shellitem_p.h"
#include "shellselection_p.h"

ShellSelection* ShellSelectionPrivate::m_dragSelection = nullptr;
IDataObject* ShellSelectionPrivate::m_dragObject = nullptr;

ShellSelection::ShellSelection(ShellFolder* folder, const QList<ShellItem>& items, QWidget* parent /*= nullptr*/)
    : QObject(parent)
    , d(new ShellSelectionPrivate())
{
    d->q = this;
    d->m_sourceFolder = folder;
    d->m_sourceItems = items;
}

ShellSelection::~ShellSelection()
{
    delete d;
}

ShellSelectionPrivate::ShellSelectionPrivate()
    : q(nullptr)
    , m_sourceFolder(nullptr)
{
}

ShellSelectionPrivate::~ShellSelectionPrivate()
{
}

ShellFolder* ShellSelection::folder() const
{
    return d->m_sourceFolder;
}

QList<ShellItem> ShellSelection::items() const
{
    return d->m_sourceItems;
}

bool ShellSelection::canTransferTo(ShellFolder* targetFolder, TransferType /*type*/)
{
    bool result = false;

    ITransferSource* source;
    HRESULT hr = d->m_sourceFolder->d->m_folder->CreateViewObject((HWND)parent()->effectiveWinId(), IID_PPV_ARGS(&source));

    if (SUCCEEDED(hr))
    {
        source->Release();

        ITransferDestination* destination;
        hr = targetFolder->d->m_folder->CreateViewObject((HWND)parent()->effectiveWinId(), IID_PPV_ARGS(&destination));

        if (SUCCEEDED(hr))
        {
            destination->Release();

            result = true;
        }
    }

    return result;
}

bool ShellSelection::transferTo(ShellFolder* targetFolder, TransferType type, Flags flags, QStringList newNames)
{
    bool result = false;

    IFileOperation* fileOperation;
    HRESULT hr = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&fileOperation));

    if (SUCCEEDED(hr))
    {
        fileOperation->SetOwnerWindow((HWND)parent()->effectiveWinId());

        DWORD operationFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMMKDIR | FOFX_NOMINIMIZEBOX;

        if (flags.testFlag(ForceOverwrite)) operationFlags |= FOF_NOCONFIRMATION;

        fileOperation->SetOperationFlags(operationFlags);

        bool sameTarget = targetFolder->isEqual(d->m_sourceFolder);

        IShellItem* targetItem;
        hr = SHCreateItemFromIDList(targetFolder->d->m_pidl, IID_PPV_ARGS(&targetItem));

        if (SUCCEEDED(hr))
        {
            for (int i = 0; i < d->m_sourceItems.count(); i++)
            {
                IShellItem* item;
                hr = SHCreateItemWithParent(nullptr, d->m_sourceFolder->d->m_folder, d->m_sourceItems.at(i).d->m_pidl, IID_PPV_ARGS(&item));

                if (SUCCEEDED(hr))
                {
                    if (type == Copy)
                        hr = fileOperation->CopyItem(item, targetItem, (LPCWSTR)newNames.at(i).utf16(), nullptr);
                    else if (sameTarget)
                        hr = fileOperation->RenameItem(item, (LPCWSTR)newNames.at(i).utf16(), nullptr);
                    else
                        hr = fileOperation->MoveItem(item, targetItem, (LPCWSTR)newNames.at(i).utf16(), nullptr);

                    item->Release();
                }

                if (FAILED(hr)) break;
            }

            if (SUCCEEDED(hr))
            {
                hr = fileOperation->PerformOperations();

                if (SUCCEEDED(hr)) result = true;
            }

            targetItem->Release();
        }

        fileOperation->Release();
    }

    return result;
}

bool ShellSelection::canDragDropTo(ShellFolder* targetFolder, TransferType type)
{
    return d->dragDropHelper(targetFolder, type, false);
}

bool ShellSelection::dragDropTo(ShellFolder* targetFolder, TransferType type)
{
    return d->dragDropHelper(targetFolder, type, true);
}

bool ShellSelectionPrivate::dragDropHelper(ShellFolder* targetFolder, ShellSelection::TransferType type, bool doDrop)
{
    bool result = false;

    QVector<LPCITEMIDLIST> pidls(m_sourceItems.count());
    for (int i = 0; i < m_sourceItems.count(); i++)
        pidls[i] = m_sourceItems.at(i).d->m_pidl;

    IDataObject* dataObject;
    HRESULT hr = m_sourceFolder->d->m_folder
                     ->GetUIObjectOf((HWND)q->parent()->effectiveWinId(), pidls.count(), pidls.data(), IID_IDataObject, nullptr, (void**)&dataObject);

    if (SUCCEEDED(hr))
    {
        IDropTarget* dropTarget;
        hr = targetFolder->d->m_folder->CreateViewObject((HWND)q->parent()->effectiveWinId(), IID_PPV_ARGS(&dropTarget));

        if (SUCCEEDED(hr))
        {
            POINTL point = {0, 0};

            DWORD possibleEffect = (type == ShellSelection::Move) ? DROPEFFECT_MOVE : DROPEFFECT_COPY;

            DWORD dragEffect = possibleEffect;
            hr = dropTarget->DragEnter(dataObject, MK_LBUTTON, point, &dragEffect);

            if (SUCCEEDED(hr) && (dragEffect & possibleEffect) != 0)
            {
                if (doDrop)
                {
                    DWORD dropEffect = possibleEffect;
                    hr = dropTarget->Drop(dataObject, MK_LBUTTON, point, &dropEffect);

                    if (SUCCEEDED(hr))
                    {
                        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST, targetFolder->d->m_pidl, nullptr);
                        result = true;
                    }
                }
                else
                {
                    dropTarget->DragLeave();
                    result = true;
                }
            }

            dropTarget->Release();
        }
    }

    return result;
}

bool ShellSelection::canDelete()
{
    bool result = false;

    ITransferSource* source;
    HRESULT hr = d->m_sourceFolder->d->m_folder->CreateViewObject((HWND)parent()->effectiveWinId(), IID_PPV_ARGS(&source));

    if (SUCCEEDED(hr))
    {
        result = true;
        source->Release();
    }

    return result;
}

bool ShellSelection::deleteSelection(Flags flags)
{
    bool result = false;

    IFileOperation* fileOperation;
    HRESULT hr = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&fileOperation));

    if (SUCCEEDED(hr))
    {
        fileOperation->SetOwnerWindow((HWND)parent()->effectiveWinId());

        DWORD operationFlags = FOF_NOCONFIRMMKDIR | FOFX_NOMINIMIZEBOX;

        if (!flags.testFlag(DeletePermanently)) operationFlags |= FOF_ALLOWUNDO;
        if (flags.testFlag(ForceDelete)) operationFlags |= FOF_NOCONFIRMATION;

        fileOperation->SetOperationFlags(operationFlags);

        for (int i = 0; i < d->m_sourceItems.count(); i++)
        {
            IShellItem* item;
            hr = SHCreateItemWithParent(nullptr, d->m_sourceFolder->d->m_folder, d->m_sourceItems.at(i).d->m_pidl, IID_PPV_ARGS(&item));

            if (SUCCEEDED(hr))
            {
                hr = fileOperation->DeleteItem(item, nullptr);

                item->Release();
            }

            if (FAILED(hr)) break;
        }

        if (SUCCEEDED(hr))
        {
            hr = fileOperation->PerformOperations();

            if (SUCCEEDED(hr)) result = true;
        }

        fileOperation->Release();
    }

    return result;
}

class ContextMenuGlobal
{
public:
    ContextMenuGlobal();
    ~ContextMenuGlobal();

public:
    IContextMenu2* m_menu;
    WNDPROC m_oldProc;
};

ContextMenuGlobal::ContextMenuGlobal()
    : m_menu(nullptr)
    , m_oldProc(nullptr)
{
}

ContextMenuGlobal::~ContextMenuGlobal()
{
}

Q_GLOBAL_STATIC(ContextMenuGlobal, contextMenuGlobal)

static LRESULT CALLBACK ContextMenuProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    ContextMenuGlobal* g = contextMenuGlobal();

    switch (message)
    {
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
        if (!wparam)
        {
            g->m_menu->HandleMenuMsg(message, wparam, lparam);
            return 0;
        }
        break;

    case WM_INITMENUPOPUP:
        g->m_menu->HandleMenuMsg(message, wparam, lparam);
        return TRUE;

    default:
        break;
    }

    return CallWindowProc(g->m_oldProc, window, message, wparam, lparam);
}

ShellSelection::MenuCommand ShellSelection::showContextMenu(const QPoint& pos, Flags flags)
{
    MenuCommand result = NoCommand;

    ContextMenuGlobal* g = contextMenuGlobal();

    QVector<LPCITEMIDLIST> pidls(d->m_sourceItems.count());
    for (int i = 0; i < d->m_sourceItems.count(); i++)
        pidls[i] = d->m_sourceItems.at(i).d->m_pidl;

    IContextMenu* contextMenu;
    HRESULT hr = d->m_sourceFolder->d->m_folder
                     ->GetUIObjectOf((HWND)parent()->effectiveWinId(), pidls.count(), pidls.data(), IID_IContextMenu, nullptr, (void**)&contextMenu);

    if (SUCCEEDED(hr))
    {
        HMENU menu = CreatePopupMenu();

        UINT menuFlags = CMF_EXPLORE;

        if (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier)) menuFlags |= CMF_EXTENDEDVERBS;

        if (flags.testFlag(CanRename)) menuFlags |= CMF_CANRENAME;

        hr = contextMenu->QueryContextMenu(menu, 0, 1000, 0x7ffff, menuFlags);

        if (SUCCEEDED(hr))
        {
            hr = contextMenu->QueryInterface(IID_PPV_ARGS(&g->m_menu));
            if (SUCCEEDED(hr)) g->m_oldProc = (WNDPROC)SetWindowLongPtr((HWND)parent()->effectiveWinId(), GWLP_WNDPROC, (LONG_PTR)ContextMenuProc);

            int command = TrackPopupMenu(menu, TPM_RETURNCMD, pos.x(), pos.y(), 0, (HWND)parent()->effectiveWinId(), nullptr);

            if (SUCCEEDED(hr))
            {
                SetWindowLongPtr((HWND)parent()->effectiveWinId(), GWLP_WNDPROC, (LONG_PTR)g->m_oldProc);

                g->m_menu->Release();
                g->m_menu = nullptr;
            }

            if (command > 0)
            {
                result = InternalCommand;

                char buffer[256];
                hr = contextMenu->GetCommandString(command - 1000, GCS_VERBA, nullptr, buffer, 256);

                if (SUCCEEDED(hr))
                {
                    if (flags.testFlag(CanOpen) && !_stricmp(buffer, "open"))
                        result = Open;
                    else if (flags.testFlag(CanRename) && !_stricmp(buffer, "rename"))
                        result = Rename;
                }

                if (result == InternalCommand)
                {
                    CMINVOKECOMMANDINFO info = {0};
                    info.cbSize = sizeof(info);
                    info.hwnd = (HWND)parent()->effectiveWinId();
                    info.lpVerb = MAKEINTRESOURCEA(command - 1000);
                    info.nShow = SW_SHOWNORMAL;

                    hr = contextMenu->InvokeCommand(&info);

                    if (FAILED(hr)) result = NoCommand;
                }
            }
        }

        DestroyMenu(menu);

        contextMenu->Release();
    }

    return result;
}

bool ShellSelection::invokeCommand(const char* verb)
{
    bool result = false;

    QVector<LPCITEMIDLIST> pidls(d->m_sourceItems.count());
    for (int i = 0; i < d->m_sourceItems.count(); i++)
        pidls[i] = d->m_sourceItems.at(i).d->m_pidl;

    IContextMenu* contextMenu;
    HRESULT hr = d->m_sourceFolder->d->m_folder
                     ->GetUIObjectOf((HWND)parent()->effectiveWinId(), pidls.count(), pidls.data(), IID_IContextMenu, nullptr, (void**)&contextMenu);

    if (SUCCEEDED(hr))
    {
        CMINVOKECOMMANDINFO info = {0};
        info.cbSize = sizeof(info);
        info.hwnd = (HWND)parent()->effectiveWinId();
        info.lpVerb = verb;
        info.nShow = SW_SHOWNORMAL;

        hr = contextMenu->InvokeCommand(&info);

        if (SUCCEEDED(hr)) result = true;

        contextMenu->Release();
    }

    return result;
}

class ShellDropSource : public IDropSource
{
public:
    ShellDropSource();
    ~ShellDropSource();

public: // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID iid, void** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

public: // IDropSource methods
    STDMETHOD(QueryContinueDrag)(BOOL escapePressed, DWORD keyState);
    STDMETHOD(GiveFeedback)(DWORD effect);

private:
    ULONG m_refs;

    Qt::MouseButtons m_currentButtons;
};

ShellDropSource::ShellDropSource()
    : m_refs(1)
    , m_currentButtons(Qt::NoButton)
{
}

ShellDropSource::~ShellDropSource()
{
}

STDMETHODIMP ShellDropSource::QueryInterface(REFIID iid, void** ppv)
{
    if (iid == IID_IUnknown || iid == IID_IDropSource)
    {
        *ppv = this;
        ++m_refs;
        return S_OK;
    }

    *ppv = nullptr;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ShellDropSource::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(ULONG) ShellDropSource::Release()
{
    if (--m_refs == 0)
    {
        delete this;
        return 0;
    }
    return m_refs;
}

static Qt::MouseButtons keyStateToMouseButtons(DWORD keyState)
{
    Qt::MouseButtons result;
    if (keyState & MK_LBUTTON) result |= Qt::LeftButton;
    if (keyState & MK_MBUTTON) result |= Qt::MidButton;
    if (keyState & MK_RBUTTON) result |= Qt::RightButton;
    if (keyState & MK_XBUTTON1) result |= Qt::XButton1;
    if (keyState & MK_XBUTTON2) result |= Qt::XButton2;
    return result;
}

STDMETHODIMP ShellDropSource::QueryContinueDrag(BOOL escapePressed, DWORD keyState)
{
    if (escapePressed) return DRAGDROP_S_CANCEL;

    if (m_currentButtons == Qt::NoButton)
    {
        m_currentButtons = keyStateToMouseButtons(keyState);
    }
    else
    {
        Qt::MouseButtons buttons = keyStateToMouseButtons(keyState);
        if (!(m_currentButtons & buttons)) return DRAGDROP_S_DROP;
    }

    QApplication::processEvents();

    return S_OK;
}

STDMETHODIMP ShellDropSource::GiveFeedback(DWORD /*effect*/)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;
}

bool ShellSelection::doDragDrop()
{
    bool result = false;

    QVector<LPCITEMIDLIST> pidls(d->m_sourceItems.count());
    for (int i = 0; i < d->m_sourceItems.count(); i++)
        pidls[i] = d->m_sourceItems.at(i).d->m_pidl;

    IDataObject* dataObject;
    HRESULT hr = d->m_sourceFolder->d->m_folder
                     ->GetUIObjectOf((HWND)parent()->effectiveWinId(), pidls.count(), pidls.data(), IID_IDataObject, nullptr, (void**)&dataObject);

    if (SUCCEEDED(hr))
    {
        ShellDropSource* dropSource = new ShellDropSource();

        ShellSelectionPrivate::m_dragSelection = this;
        ShellSelectionPrivate::m_dragObject = dataObject;

        DWORD effect = 0;
        hr = DoDragDrop(dataObject, dropSource, DROPEFFECT_COPY | DROPEFFECT_MOVE | DROPEFFECT_LINK, &effect);

        ShellSelectionPrivate::m_dragSelection = nullptr;
        ShellSelectionPrivate::m_dragObject = nullptr;

        if (SUCCEEDED(hr)) result = true;

        dropSource->Release();
        dataObject->Release();
    }

    return result;
}

ShellSelection* ShellSelection::draggedSelection(ShellDropData* data)
{
    if (data->d->m_dataObject == ShellSelectionPrivate::m_dragObject) return ShellSelectionPrivate::m_dragSelection;
    return nullptr;
}
