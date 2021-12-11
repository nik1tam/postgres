CREATE EXTENSION dummy_toaster;

CREATE TABLE tst_failed (
	t text TOASTER dummy_toaster TOASTER dummy_toaster
);

CREATE TABLE tst1 (
	t text TOASTER dummy_toaster
);

CREATE TABLE tst2 (
	t	text
);

ALTER TABLE tst2 ALTER COLUMN t SET TOASTER dummy_toaster;

-- XXX teodor: to be removed after COMMENT wiil works, because module script
-- already contains this query
COMMENT ON TOASTER dummy_toaster IS 'dummy_toaster is a dummy toaster';
