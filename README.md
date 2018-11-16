# Pyxis

## Auth flow

SignUp flow:
1. Call signUp with Google idp token
1. Verify token in server-side
1. Create User account in DB and set idp

SignIn flow:
1. Call signIn with Google idp token
1. Verify token and verify userId in DB
1. Create jwt

## JWT

JWT (Json Web Token) is used for authentication and authorization.

### Key generation

```sh
$ cd token
$ ssh-keygen -t rsa -b 4096 -f jwtRS256.key
$ openssl rsa -in jwtRS256.key -pubout -outform PEM -out jwtRS256.key.pub
```

This will generate `jwtRS256.key` and `jwtRS256.key.pub`.

### Schema

Payload:

```json
{
  user: {
    userId: "userId",
  }
}
```
