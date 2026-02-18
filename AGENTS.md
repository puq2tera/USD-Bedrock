# AGENTS.md (Bedrock Starter)

## Project overview
- This is an early-stage Bedrock-based project with a small API and plugin codebase.
- The architecture and folder layout may evolve quickly; treat current structure as guidance, not a fixed contract.
- Prefer small, incremental changes that preserve momentum while keeping behavior clear and testable.

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

## Code style and conventions
- Match naming, formatting, and structure used in surrounding files.
- Reuse existing validation and error-handling helpers where available.
- Keep logs informative and scoped; avoid noisy or redundant logging.
- Prefer clear, predictable error outputs over novel formats.
- Favor straightforward implementations that are easy to revise as requirements solidify.

## Commenting policy
- Add comments only when they materially improve understanding.
- Focus on intent, assumptions, invariants, and tradeoffs in non-obvious logic.
- Avoid comments that restate obvious code behavior.
- Prefer fewer, high-signal comments over many shallow ones.

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

## Security and safety
- Never log secrets or sensitive data.
- Keep logs and errors safe by default, especially for request payloads.
- Ask before introducing new dependencies or changing security-sensitive behavior.
