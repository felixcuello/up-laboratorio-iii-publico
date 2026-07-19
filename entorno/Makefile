all:
	@echo ""
	@echo "-------------------------------------------------------------------------------"
	@echo "                     Laboratorio III (asincrónica)"
	@echo "-------------------------------------------------------------------------------"
	@echo ""
	@echo " make build                             # construir el container"
	@echo " make shell                             # abrir una shell dentro del container"
	@echo ""
	@echo ""

IMAGE_NAME=labo3

build:
	docker compose build service1

shell:
	docker compose run --service-ports --rm -ti service1 bash
