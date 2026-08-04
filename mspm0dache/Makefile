.PHONY: all clean rebuild release verify flash

all:
	$(MAKE) -C ticlang -f makefile TEST_MODE=1 all

release:
	$(MAKE) -C ticlang -f makefile TEST_MODE=0 all

clean:
	$(MAKE) -C ticlang -f makefile clean

rebuild: clean all

verify:
	powershell.exe -NoProfile -ExecutionPolicy Bypass -File tools/verify_project.ps1

flash: all
	powershell.exe -NoProfile -ExecutionPolicy Bypass -File tools/flash_xds110.ps1
