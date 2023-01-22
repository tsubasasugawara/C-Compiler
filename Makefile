build:
	docker build -t alpine:clang .

run:
	docker run --rm -it -v $(PWD)/cc:/cc -w /cc alpine:clang

test:
	docker run --rm -v $(PWD)/cc:/cc -w /cc alpine:clang make test

rmi:
	docker rmi $$(docker images -q alpine:clang)

# make commit m=<arg>
commit:
	git add . && git commit -m "${m}"

push:
	git push -u origin main

.PHONY: build run rmi commit push
