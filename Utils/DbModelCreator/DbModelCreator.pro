TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS += \
    Core \
    DbmCreator \
    DbmCreatorUi

DbmCreator.depends = Core
DbmCreatorUi.depends = Core
DbmCreatorUi.depends = DbmCreator
