TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    Core \
    DbmCreatorUi

DbmCreatorUi.depends = Core
