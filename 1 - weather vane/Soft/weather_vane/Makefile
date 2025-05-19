.PHONY: help fmt lint clean docs
.DEFAULT_GOAL := help

INSTALL_LOCATION := ~/.local
VERSION = $(shell cat src/VERSION)

help:				## Affiche de l'aide.
	@echo "Usage: make <target>"
	@echo ""
	@echo "Targets:"
	@fgrep "##" Makefile | fgrep -v fgrep

fmt: 				## Formate les sources du projet selon les règles de codage.
	@find test/ src/  -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format -i
	@markdownlint -f **/*.md

lint:				## Lance les outils d'analyse statique du code.
	@markdownlint **/*.md || exit 1
	@find test/ src/  -iname '*.hpp' -o -iname '*.cpp' | xargs clang-format --dry-run -Werror
	@clang-tidy src/*.cpp -quiet -header-filter=.*  -- -Iinclude
	@cppcheck src/*.cpp --enable=all --inconclusive --inline-suppr --suppress=missingIncludeSystem \
		--suppress=unusedFunction --suppress=unmatchedSuppression --suppress=redundantAssignment -Iinclude

clean:          		## Nettoie les fichiers inutilisés.
	@rm -Rf public
	@rm -Rf build/

docs: 				## Génere la documentation Doxygen (HTML).
	@rm -rf public/
	doxygen Doxyfile