--------------------
DELETE FROM object_settings;
DELETE FROM object_settings_type;
--------------------

-- (obj_type, def_obj) --
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (0, 'Id', 'Ид', 'Идентификатор объекта', 'int', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (0, 'Name', 'Имя', 'Имя объекта', 'string', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (0, 'Descr', 'Описание', 'Описание объекта', 'string', '', '');
--- Server --- (1, 1)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (1, 'IP', 'IP адрес', 'IP адрес, под которым компоненты сервера будут доступны в системе', 'inet_address', '', '');
INSERT INTO object_settings(_object, key, value) VALUES (1, 'IP', '');
--INSERT INTO object_settings(_object, key, value) VALUES (1, 'AutoConnect', '1');
--- Store --- (3, 3)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (3, 'Path', 'Путь', 'Путь к объекту файловой системы, это может быть как файл, так и ссылка на диск', 'path', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (3, 'CellSize', 'Кластер', 'Размер ячейки хранилища', 'size', '16777216', '16777216');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (3, 'PageSize', 'Страница', 'Минимальный размер данных, для чтения/записи хранилища', 'size', '1048576', '1048576');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (3, 'Capacity', 'Размер хранилища', 'Размер хранилища в кластерах (как правило имеет смысл указывать только максимальное значение)', 'size', '200', 'max');
INSERT INTO object_settings(_object, key, value) VALUES (3, 'Path', 'd:/video.bin');
INSERT INTO object_settings(_object, key, value) VALUES (3, 'CellSize', '16777216');
INSERT INTO object_settings(_object, key, value) VALUES (3, 'PageSize', '1048576');
INSERT INTO object_settings(_object, key, value) VALUES (3, 'Capacity', '200');
--- Camera --- (20, 2x)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Uri', 'Uri', 'Адрес видео-источника', 'uri', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Login', 'Логин', 'Имя учётной записи для авторизации на видео-источнике', 'string', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Password', 'Пароль', 'Пароль учётной записи для авторизации на видео-источнике', 'password', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Module', 'Модуль', 'Модуль захвата RTSP', 'bool', 'live555', 'ffmpeg');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Transport', 'Транспорт', 'Транспорт передачи данных', 'bool', 'TCP', 'UDP');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (20, 'Resolution', 'Разрешение', 'Разрешение USB камеры', 'resolution', '320x240', '1920×1080');
INSERT INTO object_settings(_object, key, value) VALUES (21, 'Uri', 'rtsp://');
INSERT INTO object_settings(_object, key, value) VALUES (21, 'Login', 'admin');
INSERT INTO object_settings(_object, key, value) VALUES (21, 'Password', 'admin');
INSERT INTO object_settings(_object, key, value) VALUES (21, 'Module', '0');
INSERT INTO object_settings(_object, key, value) VALUES (21, 'Transport', '0');
INSERT INTO object_settings(_object, key, value) VALUES (22, 'Uri', 'usb://0');
INSERT INTO object_settings(_object, key, value) VALUES (22, 'Resolution', '320x240');
INSERT INTO object_settings(_object, key, value) VALUES (23, 'Uri', 'file://');
--- Anal --- (3x, 3x)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (31, 'Standby', 'Режим ожидания', 'Переход в режим ожидания при недостаточной освещённости', 'bool', 'Нет', 'Да');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (31, 'Macro', 'Макрокамера', 'Аналитика макрокамеры, т.е. камера снимает объекты очень близко', 'bool', 'Нет', 'Да');
INSERT INTO object_settings(_object, key, value) VALUES (31, 'Standby', '1');
INSERT INTO object_settings(_object, key, value) VALUES (31, 'Macro', '1');

--- Arm --- (5, 5)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (5, 'ScaleBest', 'Сохранять пропорции', 'Сохранять пропорции при выводе видео с камеры', 'bool', 'Нет', 'Да');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (5, 'ShowMouse', 'Показывать мышь', 'Показывать мышь над окном вывода с камеры', 'bool', 'Нет', 'Да');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (5, 'AutoHideMouse', 'Прятать мышь', 'Автоматически прятать мышь через короткий промежуток её не активности', 'bool', 'Нет', 'Да');
INSERT INTO object_settings(_object, key, value) VALUES (5, 'ScaleBest', '1');
INSERT INTO object_settings(_object, key, value) VALUES (5, 'ShowMouse', '1');
INSERT INTO object_settings(_object, key, value) VALUES (5, 'AutoHideMouse', '1');

--- Schedule --- (50, 51-52)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Dayly', 'Ежедневно', 'Ежедневный период работы', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly0', 'Понедельники', 'Еженедельный период работы в понедельник', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly1', 'Вторник', 'Еженедельный период работы во вторник', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly2', 'Среда', 'Еженедельный период работы в среду', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly3', 'Четверг', 'Еженедельный период работы в четверг', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly4', 'Пятница', 'Еженедельный период работы в пятницу', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly5', 'Суббота', 'Еженедельный период работы в субботу', 'time_period', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (50, 'Weekly6', 'Воскресенье', 'Еженедельный период работы в воскресенье', 'time_period', '', '');
INSERT INTO object_settings(_object, key, value) VALUES (51, 'Dayly', '8:00-23:00');

INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly0', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly1', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly2', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly3', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly4', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly5', '9:00-20:00');
INSERT INTO object_settings(_object, key, value) VALUES (52, 'Weekly6', '');

--- User --- (18, 18|28)
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (18, 'UserLogin', 'Логин', 'Логин пользователя', 'string', '', '');
INSERT INTO object_settings_type(_otype, key, name, descr, type, min_value, max_value) VALUES (18, 'UserPassword', 'Пароль', 'Пароль к логину пользователя', 'string', '', '');
INSERT INTO object_settings(_object, key, value) VALUES (18, 'UserLogin', 'root');
INSERT INTO object_settings(_object, key, value) VALUES (18, 'UserPassword', 'root');

INSERT INTO object_settings(_object, key, value) VALUES (28, 'UserLogin', 'root');
INSERT INTO object_settings(_object, key, value) VALUES (28, 'UserPassword', 'root');
