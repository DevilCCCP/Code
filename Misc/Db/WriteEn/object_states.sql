--------------------
--
-- !!! If edit: Update ObjectState.h !!!
--
--------------------
--DELETE FROM object_state;
DELETE FROM object_state_values;
DELETE FROM object_state_type;
--------------------
INSERT INTO object_state_type(_id, name, descr) VALUES (1, 'power', 'Power');

INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (1, 0, 'Off', 'gray');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (1, 1, 'On', 'green');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (1, 2, 'Stand by', 'gray');

INSERT INTO object_state_type(_id, name, descr) VALUES (2, 'service', 'Service');

INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, -2, 'Errors', 'red');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, -1, 'Warnings', 'orange');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, 0, 'Off', 'gray');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (2, 1, 'Good', 'green');

INSERT INTO object_state_type(_id, name, descr) VALUES (3, 'connect', 'Connection');

INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (3, -1, 'Not available', 'red');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (3, 0, 'Off', 'gray');
INSERT INTO object_state_values(_ostype, state, descr, color) VALUES (3, 1, 'Available', 'green');
