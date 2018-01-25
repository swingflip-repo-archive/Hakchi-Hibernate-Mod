
all: out/hibernate_mod_lite_v1.hmod

out/hibernate_mod_lite_v1.hmod:
	mkdir -p out/
	cd Hibernate_Mod/; tar -czvf "../$@" *
	touch "$@"

clean:
	-rm -rf out/

.PHONY: clean