# SofaBounds Checkpoint Usage Example

This document provides a step-by-step example of using the checkpoint feature.

## Scenario: Long-Running Calculation with Checkpointing

Suppose you want to run a calculation that might take hours or days, and you want to be able to resume it if something goes wrong.

### Step 1: Set Up Your Calculation

Start SofaBounds:
```bash
$ ./SofaBounds
SofaBounds version 1.0

Type "help" for instructions.

>
```

Configure your problem:
```
> setcorr 3
> setslope 1 7 24 25
> setslope 2 33 56 65
> setslope 3 119 120 169
> setfinalmin 56 33 65
> setfinalmax 24 7 25
```

### Step 2: Enable Checkpointing

Configure automatic checkpointing every 1000 iterations:
```
> setcheckpoint 1000 iter my_calculation.ckpt
Checkpointing enabled: every 1000 iterations to 'my_calculation.ckpt'
```

Optional: Enable progress reporting:
```
> reportevery 10 sec
> reportevery 1000 iter
```

### Step 3: Save Your Configuration

Save the configuration (including checkpoint settings) for future reference:
```
> save my_config.txt
```

### Step 4: Start the Calculation

```
> run
<iterations=0>
```

The calculation will now:
- Run the branch-and-bound algorithm
- Automatically save a checkpoint every 1000 iterations to `my_calculation.ckpt`
- Display progress updates as configured

Example output:
```
<iterations=1000 | upper bound=2.228 | time=0:01:15.432>
<iterations=2000 | upper bound=2.215 | time=0:02:31.876>
<iterations=3000 | upper bound=2.198 | time=0:03:47.123>
```

### Step 5: Interruption (Simulated or Real)

If the program is stopped (either by you pressing Ctrl+C, a crash, or power outage), you'll see:
```
> stop
stopped after 3456 iterations
lower bound  1.893117951427195 (6283362327082057/3319054854635520000)
upper bound  2.196543211234567 (...)
iterations: 3456
time: 0:04:15.789
```

### Step 6: Resume the Calculation

Later, restart SofaBounds and load your configuration:
```bash
$ ./SofaBounds
SofaBounds version 1.0

Type "help" for instructions.

> load my_config.txt
File 'my_config.txt' loaded successfully.
```

Verify checkpointing is enabled:
```
> settings

Number of corridors: 3

Slope 1:		7	24	25		(angle: 16.26020... deg)
Slope 2:		33	56	65		(angle: 30.51111... deg)
Slope 3:		119	120	169		(angle: 44.76495... deg)
Minimum final slope:	56	33	65		(angle: 59.48889... deg)
Maximum final slope:	24	7	25		(angle: 73.73979... deg)

Reporting progress every:		10 seconds
				1000 iterations

Checkpointing enabled:		every 1000 iterations
Checkpoint file:		my_calculation.ckpt
```

Now run again:
```
> run
Loaded checkpoint from 'my_calculation.ckpt'
Resuming from iteration 3000
<iterations=3000 | upper bound=2.198 | time=0:03:47.123>
```

The calculation continues from iteration 3000 (the last checkpoint), not from 0!

### Step 7: Continue Until Completion

The calculation will continue, automatically checkpointing every 1000 iterations:
```
<iterations=4000 | upper bound=2.185 | time=0:05:03.456>
<iterations=5000 | upper bound=2.172 | time=0:06:19.789>
...
```

## Tips and Best Practices

1. **Choose appropriate checkpoint frequency:**
   - More frequent = less work lost if interrupted, but more I/O overhead
   - Less frequent = better performance, but more work to redo if interrupted
   - Good starting point: 1000-5000 iterations depending on iteration speed

2. **Monitor disk space:**
   - Checkpoint files can be large (MB to GB) for complex problems
   - Only one checkpoint file is kept (it's overwritten each time)

3. **Use meaningful filenames:**
   - Include problem description: `thm9-bound1.ckpt`
   - Include date if running multiple experiments: `experiment-2024-12-18.ckpt`

4. **Save configuration files:**
   - Always use `save config.txt` to preserve your settings
   - This makes it easy to resume with the same checkpoint configuration

5. **Verify checkpoint settings:**
   - Use `settings` command to verify checkpoint is properly configured
   - Check that the checkpoint file exists: `ls -lh *.ckpt`

## Troubleshooting

**Q: The calculation started from iteration 0 instead of resuming!**
- Check that checkpointing is enabled: `settings`
- Verify the checkpoint file exists: `ls -lh my_calculation.ckpt`
- Make sure the checkpoint filename matches what you configured

**Q: How do I know if a checkpoint was saved?**
- Check for the checkpoint file: `ls -lh *.ckpt`
- The file timestamp will update each time a checkpoint is saved

**Q: Can I use the same checkpoint with different settings?**
- No! The checkpoint contains the complete problem specification
- Changing settings requires starting a new calculation with a different checkpoint file

**Q: What happens if the checkpoint file gets corrupted?**
- The calculation will start from iteration 0
- Keep backup copies of important checkpoint files if needed
