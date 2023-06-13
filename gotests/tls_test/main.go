package main

import (
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"math/rand"
	"os"
	"sync"
	"time"
)

func init() {
	rand.Seed(time.Now().Unix())
}

func main() {
	/*net.Dial("tcp", "127.0.0.1:90")*/
	println("started")
	cpool := x509.NewCertPool()
	bytes, err := os.ReadFile("/data/cimserver/certs/rootCA.crt")
	if err != nil {
		panic(err)
		return
	}
	ret := cpool.AppendCertsFromPEM(bytes)
	println(ret)
	for m := 0; m < 10000; m++ {
		go func(index int) {
			cn, err := tls.Dial("tcp", "127.0.0.1:90", &tls.Config{
				RootCAs: cpool,
			})
			if err != nil {
				panic(err)
			}

			for i := 0; i < 10000; i++ {
				_, err := cn.Write([]byte(fmt.Sprintf("%d", rand.Int63())))
				if err != nil {
					panic(err)
				}
				//println(count)

				buf := make([]byte, 1024)
				n, err := cn.Read(buf)
				if err != nil {
					panic(err)
				}
				fmt.Printf("%d----%s\n", index, string(buf[0:n]))
				time.Sleep(time.Second * 1)
			}
		}(m)
	}

	var guad sync.WaitGroup
	guad.Add(1)
	guad.Wait()
}
