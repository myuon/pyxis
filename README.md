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

## Google login

Place `google.json` file (which includes `.web.client_id` information) in `/token`

## JWT

JWT (Json Web Token) is used for authentication and authorization.

### Key generation

```sh
$ cd token
$ ssh-keygen -t ecdsa -b 256 -f jwtES256.key
$ openssl ec -in jwtES256.key -pubout -outform PEM -out jwtES256.key.pub
```

This will generate `jwtES256.key` and `jwtES256.key.pub`.

### Schema

Payload:

```json
{
  user: {
    userId: "userId",
  }
}
```

## Build

- Generates keys (google auth token/jwt keys) explained above
- Run `sls dynamodb install` before starting serverless offline server
- Set `aws credentials` for awscli
