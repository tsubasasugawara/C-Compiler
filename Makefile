build:
	docker build -t alpine:exec-c .

run:
	docker run --rm -it -v $(PWD)/cc:/cc -w /cc alpine:exec-c
