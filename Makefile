protos:
	protoc -I ./protosources/ --cpp_out=./protocompiled ./protosources/*.proto
	protoc -I ./protosources/ --grpc_out=./protocompiled --plugin=protoc-gen-grpc=/usr/bin/grpc_cpp_plugin ./protosources/*.proto
push:
	sh commit_code.sh
pull:
	git pull origin master
