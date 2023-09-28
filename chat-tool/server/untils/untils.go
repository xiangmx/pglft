package untils

import (
	"encoding/binary"
	"encoding/json"
	"fmt"
	"go-work/chat-tool/conmon/massage"
	"net"
)

type Transfor struct {
	Conn net.Conn
	Buf  [8989]byte
}

//读取包的内容并解码

func (this *Transfor) Readpkg() (mes massage.LoginType, err error) {
	//fmt.Printf("服务器等待客户端%s发信息中\n", conn.RemoteAddr())
	_, err = this.Conn.Read(this.Buf[:4])
	if err != nil {
		//fmt.Println("服务端接收数据失败,err=", err)
		return
	}
	var pkglen uint32
	pkglen = binary.BigEndian.Uint32(this.Buf[0:4])
	n, err := this.Conn.Read(this.Buf[0:pkglen])
	if n != int(pkglen) || err != nil {
		//fmt.Println("读取包内容失败,err=", err)
		return
	}
	err = json.Unmarshal(this.Buf[:pkglen], &mes)
	if err != nil {
		fmt.Println("包反序列化失败,err=", err)
		return
	}
	return
}

//发送内容

func (this *Transfor) WritePkg(types []byte) (err error) {
	var uselen uint32
	uselen = uint32(len(types))
	//fmt.Println("发送包长度为：", uselen)
	var bys [4]byte
	binary.BigEndian.PutUint32(bys[:4], uselen)
	n, err := this.Conn.Write(bys[:4])
	if n != 4 || err != nil {
		fmt.Println("conn.Write err=", err)
		return
	}
	//fmt.Println("发送包长度成功")
	_, err = this.Conn.Write(types)
	if err != nil {
		fmt.Println("conn.Write(types),err=", err)
		return
	}
	fmt.Println("发送包内容成功")
	return
}
