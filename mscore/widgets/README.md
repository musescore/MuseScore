Widgets
=======

The files in this directory define drop-in replacements for Qt's built-in
widgets. These can be [used in QtCreator's Design mode][promoting-widgets]
(i.e. in `.ui` files) or in ordinary `.h` and `.cpp` source files.

[promoting-widgets]: http://doc.qt.io/qt-5/designer-using-custom-widgets.html#promoting-widgets

## When to use

Consider using or adapting these widgets if you can rather than creating your
own. Doing this has the following benefits:

- Saves time and effort
- Easier to maintain
- Ensures consistency in design
- Reduces MuseScore's size and memory footprint
- Benefit from extra features that you may not know how to implement yourself
  (e.g. accessibility).

If none of the existing widgets suit your particular need then consider
creating your own and adding it to this directory. You can use one of the
existing ones as a template, and even inherit from it if appropriate. Try to
give the functions and classes names that are as generic as possible. Make
sure you add comments where appropriate to help other people understand how
to use the widgets, and to provide any details about the implementation that
are not immediately obvious from the code.

## Example

`searchbox.h` defines SearchBox, a drop-in replacement for a QLineEdit, and
`filterabletreeview.h` defines FilterableTreeView, which is a drop-in
replacement for a QTreeView. When used together, these classes allow you to
create a tree view to display hierarchical lists of items (e.g. books grouped
by genre) and use the line edit to search in the view.

You can use SearchBox and FilterableTreeView inside any of MuseScore's `.ui`
files like this:

1. Open the UI file (e.g. `startcenter.ui`) in Qt Creator's Design mode.
2. Drag an instance of each of the base classes (QLineEdit and QTreeView) onto
  the canvas (or find an existing instance).
3. Right-click the QLineEdit, choose "Promote to...", and enter the following:
    - Promoted class name: `Ms::SearchBox` (`Ms` is the namespace)
    - Header file: `widgets/searchbox.h`
4. Now do the same for the QTreeView:
    - Promoted class name: `Ms::FilterableTreeView`
    - Header file: `widgets/filterabletreeview.h`
5. Find the widgets' object names in the properties panel on the right.
    - These are the names you will use to refer to the widgets in the code.
    - Defaults are `treeView` and `lineEdit`. You might want to change them to
      something more easily identifiable (e.g. `bookTree` and `bookSearch`).

For many widgets that is all you need to do, but some will require additional
setting-up to be done in the UI file's corresponding code files (in this case
`startcenter.h` and `startcenter.cpp`). Look in those files for the line
`setupUi(this);` and do whatever you have to do immediately after that line.
The widget's source files (i.e. `searchbox.h`, `searchbox.cpp`, etc.) should
tell you what you need to do. In this case, you just need to call the
SearchBox's `setFilterableView(*view)` function with a pointer to the view
that you want to search (i.e. `bookSearch->setFilterableView(bookTree);`).
