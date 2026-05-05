# AGENTS.md (Bedrock Starter)

## Project overview
- This is an early-stage Bedrock-based project with a small API and plugin codebase.
- The architecture and folder layout may evolve quickly; treat current structure as guidance, not a fixed contract.
- Prefer small, incremental changes that preserve momentum while keeping behavior clear and testable.

## Command discovery and validation
- Prefer exact commands from the repo over assumptions. Check `README.md`, project scripts, workflow files, and manifests before inventing a command.
- Do not guess build, test, lint, or run commands when the repo already defines them.
- Before finishing a change, run the most relevant local validation available for the files you touched using the repo's standard commands.
- Prefer targeted validation over broad validation when the repo supports it. Use the narrowest check that reliably covers the changed area before escalating to larger suites.
- If the right validation command is still unclear after checking the repo, ask instead of guessing.

### Known validation commands (current repo)
- Core plugin tests: `./scripts/test-cpp.sh`
- Core plugin static analysis: `./scripts/clang-tidy.sh`
- Shell script lint: `./scripts/shellcheck.sh`
- Core plugin rebuild in VM: `./scripts/build-core-plugin.sh`
- Mobile client scripts (run in `client/`): `npm run start`, `npm run dev-client`, `npm run ios`, `npm run android`, `npm run web`

### Known setup/run/ops commands (current repo)
- Initial VM bootstrap and environment setup: `./scripts/launch.sh`
- Full VM setup/rerun from host: `multipass exec bedrock-starter -- sudo bash /bedrock-starter/scripts/setup.sh`
- SSH into VM: `multipass shell bedrock-starter`
- Service restarts from host: `multipass exec bedrock-starter -- sudo systemctl restart bedrock`, `multipass exec bedrock-starter -- sudo systemctl restart nginx`, `multipass exec bedrock-starter -- sudo systemctl restart php8.4-fpm`
- Service status checks from host: `multipass exec bedrock-starter -- systemctl status bedrock`, `multipass exec bedrock-starter -- systemctl status nginx`, `multipass exec bedrock-starter -- systemctl status php8.4-fpm`
- Log viewing: `./scripts/watch-logs.sh`, `./scripts/watch-logs.sh -s nginx`, `./scripts/watch-logs.sh -s php`
- Health checks: `multipass exec bedrock-starter -- bash -lc 'printf "Status\n" | nc -w 2 127.0.0.1 8888'`, `multipass exec bedrock-starter -- bash -lc 'curl -i -m 10 http://127.0.0.1/api/status'`

### Validation selection rules
- C++ changes under `server/core/`: run `./scripts/test-cpp.sh`; run `./scripts/clang-tidy.sh` when the change affects non-trivial logic or shared headers.
- Script changes under `scripts/`: run `./scripts/shellcheck.sh`.
- Mobile client changes under `client/`: run the narrowest relevant `npm run ...` script in `client/` that validates the changed flow.
- Cross-layer changes (for example C++ + scripts, or API + mobile): run checks for each touched layer.
- If a required tool is unavailable locally, state the exact missing tool and the command that was skipped.

### Execution context rules (host vs VM)
- Run `./scripts/build-core-plugin.sh` from the host only; it is a host wrapper that calls `multipass`.
- For service control and runtime diagnostics, prefer `multipass exec bedrock-starter -- ...` from the host.
- For CMake cache path mismatch issues, clear the relevant build dir (`server/core/.build` locally or `/opt/bedrock/server/core/.build` in VM) before retrying builds/tests.
- When API dependency/runtime issues appear (`vendor` missing, `.env` missing), prefer rerunning setup via `sudo bash /bedrock-starter/scripts/setup.sh` rather than ad-hoc manual fixes.

## Instruction priority and conflict resolution
- Apply instructions in this order:
- direct user instructions in the current thread
- this `AGENTS.md`
- conventions in the nearest relevant files
- broader repository conventions
- If instructions conflict in a way that could affect behavior, data integrity, security, or architecture, stop and ask for clarification.
- If the conflict is low-impact and reversible, proceed with a clearly stated assumption and keep the change minimal.

## Change summary output format
- After making code or configuration changes, provide a concise summary that includes:
- `Files changed`
- `What changed`
- `Commands run` and results
- `Assumptions made`
- `Risks / follow-ups`
- Keep summaries reviewer-focused and high signal. Avoid verbose narration.

## Common repo layout (if present)
- `Bedrock/`: upstream Bedrock source (usually a submodule/dependency).
- `server/core/` (or similar): custom Bedrock plugin logic and command handlers.
- `server/api/` (or similar): HTTP/API layer, request parsing, and Bedrock client calls.
- `server/config/`: runtime/service configuration templates.
- `scripts/`: local setup/build/test/lint/log helpers.
- `.github/workflows/`: CI checks and automation.

## Context discovery
- Read nearby files first and follow local patterns before introducing new abstractions.
- Prefer existing repository docs/scripts as the source of truth for setup, build, and test workflows.
- When changing behavior, identify the full path of impact (API surface, command handling, data layer, and tests) before editing.
- If you find multiple patterns, choose the simplest one that matches current direction and is easy to evolve.

## Setup and run commands
- Prefer project-provided scripts/README commands instead of ad-hoc commands.
- Treat script names and locations as discoverable and changeable; verify what currently exists before running.
- If command intent is unclear or ambiguous, ask instead of guessing.
- For API development in `server/api/`, restart `nginx` after API routing/config changes.
- For mobile development in `client/`, keep `client/.env` `EXPO_PUBLIC_API_BASE` aligned with the current VM IP.

## Scope and safety boundaries
### Always do
- Make minimal, targeted changes.
- Follow nearby conventions before introducing new abstractions.
- State assumptions clearly when uncertainty is low-impact and reversible.
- Preserve backward compatibility unless the task explicitly requires breaking changes.
- Avoid unrelated formatting, renaming, cleanup, or refactors outside the requested scope.

### Ask first
- Ambiguous requirements that affect behavior, data contracts, schema compatibility, or rollout behavior.
- Public API request/response/error shape changes when the expected contract is unclear.
- Security-sensitive behavior, permissions, auth flows, secret handling, or logging changes.
- Cross-boundary ownership questions where it is unclear whether the API, core plugin, or client should own the behavior.

### Never do without explicit permission
- Broad refactors, speculative cleanup, or feature expansion beyond the requested scope.
- Destructive actions such as hard resets, forceful data removal, or reverting unrelated user changes.
- Editing generated artifacts directly when the source-of-truth file or generation path should be changed instead.
- Adding new dependencies or changing lockfiles when dependency work was not requested.

## Clarifications and questions
- Do not guess on ambiguous requirements that could alter behavior or contracts.
- Ask before coding when uncertainty affects:
- User-visible behavior or API contract shape.
- Data/schema compatibility or migration behavior.
- Error semantics, logging, security, or deployment/runtime behavior.
- If uncertainty is low impact and reversible, proceed with a clearly stated assumption and keep scope minimal.
- If uncertainty is high impact or hard to reverse, stop and ask first.

## Dev workflow
- Favor minimal, targeted changes. Avoid broad refactors unless requested.
- Keep cross-layer behavior aligned when multiple layers are involved (for example API, command handlers, and data access).
- Update related tests when behavior changes, even in early-stage code.
- Prefer extending existing patterns over introducing brand-new frameworks.
- Avoid editing third-party/external dependency code unless explicitly requested.
- Avoid committing generated/build artifacts unless intentionally part of the task.

### Generated and vendor artifacts
- Treat these as generated/vendor and avoid manual edits unless the task explicitly targets them:
- `Bedrock/` (upstream submodule/dependency code)
- `server/core/.build/`
- `server/api/vendor/`
- `client/.expo/`, `client/ios/`, `client/android/` (except when the task explicitly requires native project changes)

### Cross-layer change completeness
- When adding, renaming, or changing a field, contract, auth rule, or data shape, trace the impact through every affected layer instead of stopping at the first compiling boundary.
- Typical layers to verify here include request parsing, API responses, Bedrock commands, table/schema changes, tests, and client request/response handling.
- If only part of the stack is intended to change, state that assumption explicitly in the final summary.

## Code style and conventions
- Match naming, formatting, and structure used in surrounding files.
- Reuse existing validation and error-handling helpers where available.
- Keep logs informative and scoped; avoid noisy or redundant logging.
- Prefer clear, predictable error outputs over novel formats.
- Favor straightforward implementations that are easy to revise as requirements solidify.

## Commenting policy

Comments are expected when they improve maintainability or prevent misreads. Do not default to commentless code when intent is non-obvious.

* Add comments when introducing non-trivial logic, constraints, or behavior that is easy to misinterpret.
* Prefer comments that explain **why** (intent, tradeoff, invariant, safety condition), not line-by-line **what**.
* Add a short comment when code correctness depends on ordering, side effects, tenant/security boundaries, migration safety, or contract assumptions.
* If you introduce dense parsing/validation, unusual branching, or non-obvious performance choices, leave a focused clarifying comment.
* If a block required careful reasoning during implementation, capture that reasoning briefly for the next engineer.
* Keep comments concise and local to the relevant block/function.
* Do not add comments for obvious control flow, language basics, or self-describing code.
* Do not add decorative banners, boilerplate JSDoc, or comments that restate names.
* Prefer a few high-signal comments over many shallow comments, but err toward adding a clarifying comment when uncertain.

### Commenting triggers (default)

Add at least one targeted comment in new/modified code when any of these apply:

* Non-obvious business rule or domain constraint.
* Workaround for legacy behavior, external dependency, or known bug.
* Important invariant/precondition not encoded in types.
* Data migration/backfill assumption or idempotency guard.
* Permission/tenant/object-key handling where misuse could leak data.

### Commenting examples

// GOOD: explains non-obvious behavior / ownership of side effects
// mutation onError handles user notification, so this branch only resets local UI state

// GOOD: explains safety invariant
// Domain IDs must be tenant-filtered before query composition to avoid cross-tenant leakage.

// BAD: obvious restatement
// Return the result
return result;

// BAD: decorative section banners add noise
// ============== Save Logic ==============

// Prefer whitespace + function extraction instead

// BAD: redundant JSDoc that restates the function name
/** Fetches items by ID */
function fetchItemsById(id: number) {}

## Data and schema conventions
- Follow existing project patterns for schema lifecycle (whether plugin-managed or migration-based).
- Keep schema/data changes explicit, reviewable, and consistent with local naming conventions.
- For compatibility-sensitive changes, call out assumptions and rollback/migration considerations.

## Validation and parsing
- Validate input early and consistently.
- Prefer strict parsing for structured values (numbers, booleans, lists/JSON) when supported by local patterns.
- Keep invalid data from reaching persistence or side-effecting operations.
- Mirror validation intent across layers when multiple entry points accept the same data.

## Performance and data handling
- Prefer data-layer filtering/aggregation over pulling large datasets into memory.
- Keep request paths efficient and avoid unnecessary round trips.
- Optimize only where it improves current behavior or reduces clear risk; avoid premature complexity.

## TODO conventions
- Use `TODO` markers sparingly and make them actionable.
- If the repo has a preferred author/tag format, follow it.
- Place TODOs at the exact follow-up location in code.

## Testing strategy
- Run the most relevant checks for touched files using project-provided scripts/commands.
- For backend/API changes, run targeted syntax/unit/integration checks appropriate to scope.
- For script changes, run shell linting where configured.
- Always state what was run; if checks were skipped, state why.

## Done means
- The requested change is implemented and scoped to the task.
- Relevant validation commands for touched areas were run and results were recorded.
- Behavior, contract, or schema implications were checked across affected layers.
- Any skipped checks are explicitly called out with a concrete reason.
- Diff was reviewed for unintended edits, secrets, and unrelated file churn.

## Review checklist
- Commands used are real repo commands and were run from the correct directory.
- Changed code follows nearby patterns and keeps interfaces/contracts stable unless the task requested a change.
- Input validation, error handling, and logging are consistent with local conventions.
- No generated/vendor files were hand-edited unless explicitly required.
- Risks, assumptions, and follow-up work are documented in the final summary.

## Extended workflows
- For AGENTS quality updates, use the `edit-agents-md` skill and `~/.codex/best_practices_for_agents.md` as the rubric.
- Keep this file focused on durable repo guidance; place repeated task-specific workflows in skills and large multi-step execution in plan artifacts.

## Security and safety
- Never log secrets or sensitive data.
- Keep logs and errors safe by default, especially for request payloads.
- Ask before introducing new dependencies or changing security-sensitive behavior.
