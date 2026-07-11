# Contributing to PAT-Noxim

Thank you for your interest in improving PAT-Noxim, a cycle-accurate Power,
Area, and Thermal Network-on-Chip (NoC) simulator. Contributions of all
kinds are welcome: bug fixes, new router/PE architectures, model
improvements, documentation, and examples.

## Getting Started

1. **Read the documentation.** The [`docs/`](docs/) folder contains an
   [INDEX](docs/INDEX.md), an [installation guide](docs/INSTALLATION.md), an
   [architecture overview](docs/ARCHITECTURE.md), a
   [developer guide](docs/DEVELOPER_GUIDE.md), and a
   [file guide](docs/FILE_GUIDE.md).
2. **Build the simulator.** Install SystemC 2.2.0 first, then build as
   described in the [README](README.md#how-to-install):

   ```
   $ cd PAT-Noxim/bin
   $ make
   $ make install
   ```
3. **Verify your setup** by running an example simulation from
   [docs/RUNNING_SIMULATIONS.md](docs/RUNNING_SIMULATIONS.md).

## Reporting Issues

When opening an issue, please include:

- A clear description of the problem or feature request.
- The configuration file and command line you used.
- Your platform, compiler version, and SystemC version.
- Expected vs. actual behavior, and any logs or error output.

For security-sensitive reports, follow [SECURITY.md](SECURITY.md) instead of
opening a public issue.

## Submitting Changes

1. Fork the repository and create a topic branch from `master`.
2. Keep each pull request focused on a single change.
3. Match the existing code style, and follow the project's tradition of
   thorough in-code comments (see the note on documentation in the
   [README](README.md)).
4. Update or add documentation in `docs/` when your change affects behavior,
   configuration parameters, or the build process.
5. Build cleanly and confirm that a baseline simulation still runs before
   submitting.
6. Write a descriptive commit message and PR description explaining the
   motivation and the approach.

## License

PAT-Noxim is released under the terms of the GNU General Public License v3.0
(see [LICENSE](LICENSE)). By contributing, you agree that your contributions
will be licensed under the same license.

## Citation

If you use PAT-Noxim in academic work, please cite:

> A. Norollah, D. Derafshi, H. Beitollahi and A. Patooghy, "PAT-Noxim: A
> Precise Power & Thermal Cycle-Accurate NoC Simulator," 2018 31st IEEE
> International System-on-Chip Conference (SOCC), Arlington, VA, USA, 2018,
> pp. 163-168. doi: 10.1109/SOCC.2018.8618491

## Contact

Questions? Reach out to
[a.norollah.official@gmail.com](mailto:a.norollah.official@gmail.com).
