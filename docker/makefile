all:
	docker build -t kaityo256/xbyak_aarch64_handson .

no-cache:
	docker build -t kaityo256/xbyak_aarch64_handson . --no-cache

run:
	docker run -e GIT_USER=${GIT_USER} -e GIT_TOKEN=${GIT_TOKEN} -it kaityo256/xbyak_aarch64_handson
