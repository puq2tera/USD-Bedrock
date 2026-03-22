# Auth Smoke Checklist

- Register a new user from `Register` screen.
- Confirm app routes to poll list after successful register.
- Force close app and reopen; confirm session stays logged in.
- Create a new poll and verify it appears in list.
- Open poll and submit a vote.
- Call `logout()` from debug console and confirm app redirects to login.
- Try poll endpoints without a bearer token and confirm `401 Unauthorized`.
