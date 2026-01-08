import { useState } from 'react'

import {
    Box,
    Button,
    LinearProgress,
    TextField,
    Typography,
} from '@mui/material'
import CloudUploadIcon from '@mui/icons-material/CloudUpload'

import axios from 'axios'

const Upload = () => {
    const [uploadProgress, setUploadProgress] = useState<number>(0)

    const handleFileUpload = async (
        event: React.FormEvent<HTMLFormElement>
    ) => {
        event.preventDefault()

        console.log('submitted')

        const form = event.currentTarget

        const fileInput: HTMLInputElement = form.elements.item(
            0
        ) as HTMLInputElement

        console.log(fileInput)

        if (!fileInput || !fileInput.files || fileInput.files.length == 0) {
            return
        }

        const uploadFile: FormData = new FormData()
        const file: File = fileInput.files[0]

        const reader = new FileReader()

        reader.onload = (e: ProgressEvent<FileReader>) => {
            const text = e.target?.result
            console.log('File content: ', text)
        }

        reader.readAsText(file)

        uploadFile.append('upload_file', file)

        try {
            await axios.post('http://127.0.0.1:8000/upload', uploadFile, {
                headers: {
                    'Content-Type': 'multipart/form-data',
                },
                onUploadProgress: (progressEvent) => {
                    const percentCompleted = Math.round(
                        (progressEvent.loaded * 100) /
                            (progressEvent.total || 1)
                    )
                    setUploadProgress(percentCompleted)
                },
            })

            console.log('Upload complete')

            setTimeout(() => setUploadProgress(0), 1000)
        } catch (error) {
            console.log(error)
            setUploadProgress(0)
        }
    }

    return (
        <Box>
            <Typography variant="h4" sx={{ fontWeight: 800 }}>
                Upload File
            </Typography>
            <form onSubmit={(event) => handleFileUpload(event)}>
                <TextField
                    type="file"
                    inputProps={{ accept: 'text/*' }}
                    fullWidth
                    margin="normal"
                />
                {uploadProgress > 0 ? (
                    <LinearProgress
                        variant="determinate"
                        value={uploadProgress}
                    />
                ) : (
                    <Button
                        variant="contained"
                        color="primary"
                        startIcon={<CloudUploadIcon />}
                        fullWidth
                        type="submit"
                    >
                        Upload
                    </Button>
                )}
            </form>
        </Box>
    )
}

export default Upload
