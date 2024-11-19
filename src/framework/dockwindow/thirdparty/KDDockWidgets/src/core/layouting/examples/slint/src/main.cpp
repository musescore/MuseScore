/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "appwindow.h"
#include "../../../Item_p.h"
#include "../../../LayoutingHost_p.h"
#include "../../../LayoutingGuest_p.h"
#include "../../../LayoutingSeparator_p.h"

#include <iostream>

using namespace KDDockWidgets;

class Separator;
class Host : public KDDockWidgets::Core::LayoutingHost
{
public:
    explicit Host(slint::ComponentHandle<AppWindow> ui);

    Separator *
    separatorForId(int id) const;

    bool supportsHonouringLayoutMinSize() const override
    {
        return true;
    }

    slint::ComponentHandle<AppWindow> ui;
};

namespace {

int index_of_row(std::string name, slint::ComponentHandle<AppWindow> ui)
{
    auto model = ui->get_dockWidgets();
    if (!model)
        return -1;

    for (int i = 0, end = model->row_count(); i < end; ++i) {
        auto dw = model->row_data(i);
        if (dw->uniqueName.data() == name) {
            return i;
        }
    }

    return -1;
}

int index_of_separator_row(int id, slint::ComponentHandle<AppWindow> ui)
{
    auto model = ui->get_separators();
    if (!model)
        return -1;

    for (int i = 0, end = model->row_count(); i < end; ++i) {
        auto dw = model->row_data(i);
        if (dw->id == id) {
            return i;
        }
    }

    return -1;
}

void dump_model(slint::ComponentHandle<AppWindow> ui)
{
    if (auto model = ui->get_dockWidgets()) {
        for (int i = 0, end = model->row_count(); i < end; ++i) {
            std::cout << "i=" << i << "name=" << model->row_data(i)->uniqueName.data() << "\n";
        }
    } else {
        std::cerr << "null model\n";
    }
}

void update_model_row(DockWidgetDescriptor desc, slint::ComponentHandle<AppWindow> ui)
{
    const int index = index_of_row(desc.uniqueName.data(), ui);
    if (index == -1) {
        std::cerr << "Failed to find descriptor for: " << desc.uniqueName << "; count is: " << ui->get_dockWidgets()->row_count() << "\n";
    } else {
        auto model = ui->get_dockWidgets();
        assert(model);
        model->set_row_data(index, desc);
    }
}

void add_model_row(DockWidgetDescriptor desc, slint::ComponentHandle<AppWindow> ui)
{
    const int index = index_of_row(desc.uniqueName.data(), ui);
    if (index != -1) {
        std::cerr << "add_model_row: Row already exists!\n";
        return;
    }

    if (auto model = ui->get_dockWidgets()) {
        auto vecModel = std::static_pointer_cast<slint::VectorModel<DockWidgetDescriptor>>(model);
        vecModel->push_back(desc);
    } else {
        std::vector<DockWidgetDescriptor> dockDescriptors = {
            desc
        };

        ui->set_dockWidgets(std::make_shared<slint::VectorModel<DockWidgetDescriptor>>(dockDescriptors));
    }
}

void add_separator_model_row(SeparatorDescriptor desc, slint::ComponentHandle<AppWindow> ui)
{
    auto separatorModel = ui->get_separators();
    if (separatorModel) {
        auto vecModel = std::static_pointer_cast<slint::VectorModel<SeparatorDescriptor>>(separatorModel);
        vecModel->push_back(desc);
    } else {
        std::vector<SeparatorDescriptor> sepDescriptors { desc };
        ui->set_separators(std::make_shared<slint::VectorModel<SeparatorDescriptor>>(sepDescriptors));
    }
}

void update_separator_model_row(SeparatorDescriptor desc, slint::ComponentHandle<AppWindow> ui)
{
    const int index = index_of_separator_row(desc.id, ui);
    if (index == -1) {
        std::cerr << "Failed to find separator descriptor for: " << desc.id << "; count is: " << ui->get_separators()->row_count() << "\n";
    } else {
        auto model = ui->get_separators();
        assert(model);
        model->set_row_data(index, desc);
    }
}

void remove_separator_model_row(int id, slint::ComponentHandle<AppWindow> ui)
{
    const int index = index_of_separator_row(id, ui);
    if (index == -1) {
        std::cerr << "remove_separator_model_row: Failed to find separator descriptor for: " << id << "; count is: " << ui->get_separators()->row_count() << "\n";
    } else {
        auto vecModel = std::static_pointer_cast<slint::VectorModel<SeparatorDescriptor>>(ui->get_separators());
        vecModel->erase(index);
    }
}

}

class Guest : public KDDockWidgets::Core::LayoutingGuest
{
public:
    explicit Guest(Host *host, const QString &uniqueName, slint::ComponentHandle<AppWindow> ui)
        : _uniqueName(uniqueName)
        , ui(ui)
    {
        auto item = new Core::Item(host);
        item->setGuest(this);
    }

    Size minSize() const override
    {
        return { 100, 100 };
    }

    Size maxSizeHint() const override
    {
        return { 1000, 1000 };
    }

    void setGeometry(Rect r) override
    {
        if (r == _geometry)
            return;

        _geometry = r;
        update_model_row(descriptor(), ui);
    }

    DockWidgetDescriptor descriptor() const
    {
        return DockWidgetDescriptor { _isVisible, slint::SharedString(_uniqueName), float(_geometry.x()), float(geometry().y()), float(_geometry.width()), float(_geometry.height()) };
    }

    void setVisible(bool is) override
    {
        if (_isVisible == is)
            return;

        _isVisible = is;
        update_model_row(descriptor(), ui);
    }

    Rect geometry() const override
    {
        return _geometry;
    }

    void setHost(Core::LayoutingHost *parent) override
    {
        if (_layoutingHost == parent)
            return;

        add_model_row(descriptor(), ui);
        _layoutingHost = parent;
    }

    Core::LayoutingHost *host() const override
    {
        return _layoutingHost;
    }

    QString id() const override
    {
        return _uniqueName;
    }

    Core::LayoutingHost *_layoutingHost = nullptr;
    QString _uniqueName;
    slint::ComponentHandle<AppWindow> ui;
    Rect _geometry;
    bool _isVisible = false;
};

class Separator : public Core::LayoutingSeparator
{
public:
    explicit Separator(Core::LayoutingHost *host, Qt::Orientation orientation, Core::ItemBoxContainer *container)
        : Core::LayoutingSeparator(host, orientation, container)
        , _host(static_cast<Host *>(host))
    {
        static int counter = 0;
        _id = ++counter;

        add_separator_model_row(descriptor(), this->_host->ui);
    }

    ~Separator() override
    {
        remove_separator_model_row(_id, _host->ui);
    }

    Rect
    geometry() const override
    {
        return _geo;
    }

    void setGeometry(Rect g) override
    {
        if (g == _geo)
            return;

        _geo = g;
        update_separator_model_row(descriptor(), _host->ui);
    }

    SeparatorDescriptor descriptor() const
    {
        return SeparatorDescriptor { float(_geo.x()), float(_geo.y()), float(_geo.width()), float(_geo.height()), _id, isVertical() };
    }

    Rect _geo;
    int _id = 0;
    Host *const _host;
};

Host::Host(slint::ComponentHandle<AppWindow> ui)
    : ui(ui)

{
    m_rootItem = new Core::ItemBoxContainer(this);

    ui->on_window_size_changed_callback([this](float w, float h) -> bool {
        m_rootItem->setSize_recursive({ int(w), int(h) });
        return true;
    });

    ui->on_separator_event([&](int id, slint::private_api::PointerEvent ev) {
        if (Separator *separator = separatorForId(id)) {
            if (ev.kind == slint::cbindgen_private::PointerEventKind::Down) {
                separator->onMousePress();
            } else if (ev.kind == slint::cbindgen_private::PointerEventKind::Up) {
                separator->onMouseRelease();
            }
        } else {
            std::cerr << "on_separator_event: Expected separator with id=" << id << "\n";
        }
    });

    ui->on_separator_moved([&](int id, float x, float y) {
        if (Separator *sep = separatorForId(id)) {
            if (sep->isVertical()) {
                const int oldPos = sep->position();
                sep->onMouseMove({ 0, int(oldPos + y) });
            } else {
                const int oldPos = sep->position();
                sep->onMouseMove({ int(oldPos + x), 0 });
            }
        } else {
            std::cerr << "on_separator_moved: Expected separator with id=" << id << "\n";
        }
    });
}

Separator *Host::separatorForId(int id) const
{
    auto separators = static_cast<Core::ItemBoxContainer *>(m_rootItem)->separators_recursive();
    for (auto separator : separators) {
        auto slintSeparator = static_cast<Separator *>(separator);
        if (slintSeparator->_id == id)
            return slintSeparator;
    }

    return nullptr;
}

int main(int argc, char **argv)
{
    auto ui = AppWindow::create();
    ui->set_image(slint::Image::load_from_path(SRC_DIR "/slint-logo-full-dark-large.png"));

    /// Tell KDDW about our separators
    Core::Item::setCreateSeparatorFunc([](Core::LayoutingHost *host, Qt::Orientation orientation, Core::ItemBoxContainer *container) -> Core::LayoutingSeparator * {
        return new Separator(host, orientation, container);
    });

    Host host(ui);
    auto guest1 = new Guest(&host, "1", ui);
    auto guest2 = new Guest(&host, "2", ui);
    auto guest3 = new Guest(&host, "3", ui);
    auto guest4 = new Guest(&host, "4", ui);

    host.insertItem(guest1, KDDockWidgets::Location_OnLeft);
    host.insertItem(guest2, KDDockWidgets::Location_OnRight);
    host.insertItemRelativeTo(guest3, /*relativeTo=*/guest2, KDDockWidgets::Location_OnBottom);
    host.insertItem(guest4, KDDockWidgets::Location_OnTop, Size(0, 200));

    ui->run();
    return 0;
}
