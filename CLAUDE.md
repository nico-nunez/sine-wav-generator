# Project Context

## About This Project
- **Production-quality synthesizer development** - Building a professional-grade audio synthesizer using C++ and modern DSP techniques
- Focus: Production patterns, performance, SIMD-ready architecture, real-time audio constraints
- Learning path: Direct implementation of industry-standard techniques, not educational simplifications

## Technical Approach
- **Production-first, always** - Use patterns from professional synthesizers (Vital, Serum, etc.), not beginner shortcuts
- **Assume solid programming background** - Focus on C++ audio/DSP specifics, performance optimization, real-time constraints
- **Explain the "why" with context** - Explain rationale with references to production synthesizers when relevant
- **Performance matters** - SIMD-ready architecture, cache-friendly data structures, real-time safe code
- **Functional/procedural style preferred** - SoA + pure functions in hot paths, minimal OOP overhead in audio processing

## Working Style
- **DO NOT update files unless explicitly requested** - this is a learning project and automatic fixes defeat the purpose
- Offer suggestions, explanations, and guidance instead of making changes
- When presenting options, explain trade-offs but lean toward industry best practices
- Exception: Documentation and reference materials can be created/updated when asked
- **"Plan" means a doc** - When asked to "make a plan" or "create a plan", write a planning document in `_docs_/` (or update the roadmap). Do NOT enter plan mode.

## Documentation Style
Reference docs in `_docs_/` (note the underscores) should be:
- **Concise and to the point** - no excessive filler
- **Scannable** - clear sections, code examples, key takeaways
- **Practical** - focus on "aha moments" and common gotchas
- If it's too long or wordy, it won't get read!

### Documentation Requirements
1. **Table of Contents** - All doc files must include a table of contents with section links at the top
2. **README.md Updates** - When creating new docs, add them to the corresponding README.md file in the directory
3. **Keep Index Current** - Ensure all existing docs are listed in their directory's README.md
