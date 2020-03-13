DO $$
DECLARE
 version_new_ CONSTANT integer := 0;-- new version number --
 ver_ integer;
BEGIN
  LOCK TABLE variables;
  SELECT version_is('Base') INTO ver_;
  IF ver_ < version_new_ THEN
    -- begin update --

    -- end update --
    PERFORM version_set('Base', version_new_);
  END IF;
END$$;