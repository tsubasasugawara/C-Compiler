build:
	docker build -t alpine:exec-c .

run:
	docker run --rm -it -v $(PWD)/cc:/cc -w /cc alpine:exec-c

test:
	docker run --rm -v $PWD/cc:/cc -w /cc alpine:exec-c make test

rmi:
	docker rmi $$(docker images -q alpine:exec-c)

# make commit m=<arg>
commit:
	git add . && git commit -m "${m}"

push:
	git push -u origin main

.PHONY: build run rmi commit push
