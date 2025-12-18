# Checkpoint Feature Documentation

## Overview

The checkpoint feature allows SofaBounds to automatically save its search state at regular intervals during computation. If the program crashes or there is a power outage, the computation can be resumed from the last checkpoint by simply running the program again with the same settings.

## Usage

### Enabling Checkpoints

Use the `setcheckpoint` command to enable automatic checkpointing:

```
setcheckpoint [iterations] iter [filename]
```

**Parameters:**
- `iterations`: Number of iterations between checkpoints (e.g., 100, 1000)
- `iter`: Keyword indicating iteration-based checkpointing
- `filename`: Path to the checkpoint file (e.g., "checkpoint.dat")

**Example:**
```
> setcheckpoint 1000 iter checkpoint.dat
```

This will save the search state every 1000 iterations to a file named "checkpoint.dat".

### Disabling Checkpoints

To disable checkpointing:

```
> setcheckpoint 0 iter
```

### Resuming from Checkpoint

When checkpointing is enabled and you run the calculation, the program automatically:
1. Checks if the checkpoint file exists
2. If it exists, loads the saved state and resumes computation
3. If it doesn't exist, starts a fresh calculation

Simply use the `run` command as usual:

```
> run
```

If a checkpoint exists, you'll see:
```
Loaded checkpoint from 'checkpoint.dat'
Resuming from iteration 1000
```

### Checkpoint File Contents

The checkpoint file contains:
- Problem specification (corridors, slopes, etc.)
- Current upper and lower bounds
- Iteration count
- Elapsed time
- Priority queue state (all pending boxes to explore)
- Lower bound witness and polygons

**Note:** Checkpoint files are in a special binary format and should not be edited manually.

## Example Workflow

1. Load settings and enable checkpointing:
```
> load example-45.txt
> setcheckpoint 1000 iter checkpoint.dat
> run
```

2. Let the calculation run. It will automatically save every 1000 iterations.

3. If the program stops (crash, power outage, or you stop it manually):
```
> stop
```

4. To resume later, just restart the program and run again with checkpoint enabled:
```
> setcheckpoint 1000 iter checkpoint.dat
> run
```

The calculation will resume from the last checkpoint.

## Tips

- Choose checkpoint frequency based on your needs:
  - More frequent checkpoints (e.g., every 100 iterations) = less work lost if crash, but more I/O overhead
  - Less frequent checkpoints (e.g., every 10000 iterations) = less I/O overhead, but more work lost if crash
  
- The checkpoint file can be large for complex problems with many boxes in the priority queue

- You can include checkpoint settings in your configuration files saved with the `save` command

- Check the current checkpoint settings with the `settings` command

## Integration with Other Commands

The checkpoint feature integrates seamlessly with existing commands:

- `settings`: Shows current checkpoint configuration
- `save [filename]`: Includes checkpoint settings in the saved configuration
- `load [filename]`: Can load checkpoint settings from saved configurations
