Running python example
======================

Generate resource file with:
~# rcc -g python -o rc_assets.py ../../examples/dockwidgets/resources_example.qrc
(on some systems, rcc might be invoked as rcc-qt5)

Make sure you installed kddockwidgets and set PYTHONPATH accordingly.

Run the app:
~# python3 main.py
