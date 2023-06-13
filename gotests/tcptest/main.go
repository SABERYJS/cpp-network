package main

import (
	"math/rand"
	"net"
	"sync"
	"time"
)

func init() {
	rand.Seed(time.Now().UnixNano())
}

var letterRunes = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")

func RandStringRunes(n int) string {
	b := make([]rune, n)
	for i := range b {
		b[i] = letterRunes[rand.Intn(len(letterRunes))]
	}
	return string(b)
}

func main() {
	for i := 0; i < 1000; i++ {
		go func() {
			conn, err := net.Dial("tcp", "127.0.0.1:90")
			if err != nil {
				println(err.Error())
				return
			}

			buf:=make([]byte,1024)
			for m := 0; m < 10; m++ {
				_, err := conn.Write([]byte(RandStringRunes(30)))
				if err != nil {
					panic(err.Error())
				}
				_,err=conn.Read(buf)
				if err!=nil{
					panic(err.Error())
				}
			}
			//conn.Close()
			time.Sleep(time.Second*100)
		}()
	}
	var yg sync.WaitGroup
	yg.Add(1)
	yg.Wait()
}
