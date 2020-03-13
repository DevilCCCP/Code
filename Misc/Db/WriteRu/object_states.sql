--------------------
--
-- !!! If edit: Update ObjectState.h !!!
--
--------------------
--DELETE FROM object_state;
DELETE FROM object_state_values;
DELETE FROM object_state_type;
--------------------
INSERT INTO object_state_type(_id, name, descr) VALUES (1, 'power', 'Питание');

INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (1, 0, 'Выключено', 'gray');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (1, 1, 'Включено', 'green');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (1, 2, 'Спящий режим', 'gray');

INSERT INTO object_state_type(_id, name, descr) VALUES (2, 'service', 'Сервис');

INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, -2, 'Проблемы', 'red');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, -1, 'Замечания', 'orange');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, 0, 'Выключено', 'gray');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, 1, 'Порядок', 'green');

INSERT INTO object_state_type(_id, name, descr) VALUES (3, 'connect', 'Соединение');

INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (3, -1, 'Недоступно', 'red');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (3, 0, 'Выключено', 'gray');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (3, 1, 'Доступно', 'green');
