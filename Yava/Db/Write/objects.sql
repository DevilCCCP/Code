--------------------
DELETE FROM object;
DELETE FROM object_type;
--------------------
INSERT INTO object_type (_id, name, descr) VALUES (0, 'tmp', 'Шаблон');
INSERT INTO object_type (_id, name, descr) VALUES (1, 'srv', 'Сервер');
INSERT INTO object_type (_id, name, descr) VALUES (20, 'cam', 'Камера');
INSERT INTO object_type (_id, name, descr) VALUES (3, 'rep', 'Хранилище');
INSERT INTO object_type (_id, name, descr) VALUES (4, 'svc', 'Сервис');
INSERT INTO object_type (_id, name, descr) VALUES (5, 'arm', 'АРМ оператора');
INSERT INTO object_type (_id, name, descr) VALUES (50, 'sch', 'Расписание');

INSERT INTO object_type (_id, name, descr) VALUES (31, 'va1', 'Видеоаналитика движения v1');

INSERT INTO object_type (_id, name, descr) VALUES (18, 'usr', 'Пользователь');
INSERT INTO object_type (_id, name, descr) VALUES (19, 'upd', 'Точка обновления');
SELECT setval('object_type__id_seq', 100);
--------------------
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (0, 0, 'tmp', 'Шаблоны', 'Корневой объект для всех шаблонов', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (1, 1, 'srv', 'Сервер', 'Сервер по умолчанию', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (21, 20, 'cam', 'Камера', 'Камера по умолчанию', 0, 'tcp::', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (22, 20, 'usb', 'USB веб-камера', 'Веб-камера по умолчанию', 0, 'tcp::', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (23, 20, 'file', 'Видеофайл', 'Видеофайл по умолчанию', 0, 'tcp::', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (3, 3, 'rep', 'Хранилище', 'Хранилище по умолчанию', 0, '', -1);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (5, 5, 'arm', 'АРМ оператора', 'АРМ оператора по умолчанию', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (31, 31, 'va1', 'Аналитика движения v1', 'Анализ видеопотока для выявления двужущихся объектов и анализа их траекторий', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (18, 18, 'usr', 'Пользователь', 'Пользователь кластера', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (19, 19, 'upd', 'Точка обновления', 'Точка обновления по-умолчанию', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (51, 50, 'sch1', 'Ежедневное расписание', 'Расписание, одинаковое каждый день', 0, '', 0);
INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (52, 50, 'sch2', 'Еженедельное расписание', 'Расписание, одинаковое каждую неделю', 0, '', 0);

INSERT INTO object(_id, _otype, guid, name, descr, revision, uri, status) VALUES (28, 18, 'root', 'root', 'Администратор кластера', 0, '', 0);
SELECT setval('object__id_seq', 100);
--------------------
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 1, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 5, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 18, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 19, 0);

INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 51, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 52, 0);

INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 21, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 22, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 23, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 3, 0);
INSERT INTO object_connection(_omaster, _oslave, type) VALUES (0, 31, 0);
