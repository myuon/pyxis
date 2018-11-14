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

## jwt

JWT (Json Web Token) is used for authentication and authorization.

Payload:

```json
{
  user: {
    userId: "userId",
  }
}
```
