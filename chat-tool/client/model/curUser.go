package model

import (
	"go-work/chat-tool/conmon/massage"
	"net"
)

type CurUser struct {
	Conn net.Conn
	massage.User
}
