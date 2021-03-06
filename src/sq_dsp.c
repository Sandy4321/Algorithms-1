#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <fftw3.h>

#include "sq_constants.h"
#include "sq_dsp.h"
#include "sq_utils.h"

int sq_power(FILE* instream, FILE* outstream, unsigned int nsamples)
{
    float *smpls_bfr;
    unsigned int smpli;

    if ((nsamples < 2) || (nsamples >= MAX_SMPLS_LEN))
        return ERR_ARG_BOUNDS;

    smpls_bfr = malloc(nsamples * sizeof(float) * 2);
    if(smpls_bfr == NULL)
        return ERR_MALLOC;
    
    while (fread(smpls_bfr, sizeof(float) * 2, nsamples, instream) == nsamples)
    {
        for (smpli = 0; smpli < nsamples ; smpli++)
        {
            smpls_bfr[(smpli<<1)+0] =
                (smpls_bfr[(smpli<<1)+0] * smpls_bfr[(smpli<<1)+0]) +
                (smpls_bfr[(smpli<<1)+1] * smpls_bfr[(smpli<<1)+1]);
            smpls_bfr[(smpli<<1)+1] = 0.0;
        }

        fwrite(smpls_bfr, sizeof(float) * 2, nsamples, outstream);
    }

    free(smpls_bfr);

    return 0;
}

int sq_sum(FILE* instream, FILE* outstream, unsigned int nsamples)
{

    if (!((nsamples >= 2) && (nsamples <= MAX_SMPLS_LEN)))
        return ERR_ARG_BOUNDS;

    float *smpls_bfr;
    float *sum_bfr;

    smpls_bfr = malloc(nsamples * 4 * 2);
    if(smpls_bfr == NULL)
        return ERR_MALLOC;
    
    sum_bfr = malloc(nsamples * 4 * 2);
    if(smpls_bfr == NULL)
        return ERR_MALLOC;
    
    unsigned int smpli;

    for (smpli = 0; smpli < nsamples; smpli++)
    {
        sum_bfr[(smpli<<1)+0] = 0.0;
        sum_bfr[(smpli<<1)+1] = 0.0;
    }

    while (fread(smpls_bfr, 8, nsamples, instream) == nsamples)
    {
        for (smpli = 0; smpli < nsamples; smpli++)
        {
            sum_bfr[(smpli<<1)+0] += (smpls_bfr[(smpli<<1)+0] / (float) nsamples);
            sum_bfr[(smpli<<1)+1] += (smpls_bfr[(smpli<<1)+1] / (float) nsamples);
        }
    }

    fwrite(sum_bfr, 8, nsamples, outstream);

    free(sum_bfr);
    free(smpls_bfr);

    return 0;
}

int sq_window(FILE* instream, FILE* outstream, unsigned int wndw_len)
{
    float *wndw_bfr;
    float *in_bfr;
    float *out_bfr;

    if (!((wndw_len >= 2) && (wndw_len <= MAX_WNDW_LEN)))
        return ERR_ARG_BOUNDS;

    wndw_bfr = malloc(wndw_len * 4);
    if(wndw_bfr == NULL)
        return ERR_MALLOC;
    in_bfr = malloc(wndw_len * 4 * 2);
    if(in_bfr == NULL)
        return ERR_MALLOC;
    out_bfr = malloc(wndw_len * 4 * 2);
    if(out_bfr == NULL)
        return ERR_MALLOC;

    unsigned int wndwi, bfri;

    for (wndwi = 0; wndwi < wndw_len; wndwi++)
    {
        wndw_bfr[wndwi] =
            cos(((((float)wndwi) - ((wndw_len - 1.0) / 2.0)) / ((wndw_len - 1.0) / 2.0)) * (M_PI / 2.0));
    }

    fread(&in_bfr[(wndw_len/2)*2], 8, wndw_len / 2, instream);
    memcpy(&in_bfr[0], &in_bfr[(wndw_len/2)*2], (wndw_len / 2)*4*2);

    while (fread(&in_bfr[(wndw_len/2)*2], 8, wndw_len / 2, instream) == (wndw_len / 2))
    {
        for (bfri = 0; bfri < wndw_len; bfri++)
        {
            out_bfr[(bfri<<1)+0] = in_bfr[(bfri<<1)+0] * wndw_bfr[bfri];
            out_bfr[(bfri<<1)+1] = in_bfr[(bfri<<1)+1] * wndw_bfr[bfri];
        }
        fwrite(out_bfr, 8, wndw_len, outstream);
        memcpy(&in_bfr[0], &in_bfr[(wndw_len/2)*2], (wndw_len / 2)*4*2);
    }

    free(wndw_bfr);
    free(in_bfr);
    free(out_bfr);

    return 0;
}

int sq_component(FILE* instream, FILE* outstream, unsigned int nsamples, int component)
{
    if (nsamples <= 0)
        return ERR_ARG_BOUNDS;
    if ((component != 0) && (component != 1))
        return ERR_ARG_BOUNDS;

    int i;
    cmplx *sbfr;
    float *rbfr;

    sbfr = malloc(nsamples * sizeof(cmplx));
    if(sbfr == NULL)
        return ERR_MALLOC;
    rbfr = malloc(nsamples * sizeof(float));
    if(rbfr == NULL)
        return ERR_MALLOC;

    while (fread(sbfr, sizeof(cmplx), nsamples, instream) == nsamples)
    {
        for (i = 0; i < nsamples; i++)
            rbfr[i] = sbfr[i][component];
        fwrite(rbfr, sizeof(float), nsamples, outstream);
    }

    free(rbfr);
    free(sbfr);

    return 0;
}

int sq_real(FILE* instream, FILE* outstream, unsigned int nsamples)
{
    return sq_component(instream, outstream, nsamples, REAL);
}

int sq_imag(FILE* instream, FILE* outstream, unsigned int nsamples)
{
    return sq_component(instream, outstream, nsamples, IMAG);
}

int sq_fft(FILE* instream, FILE* outstream, unsigned int fft_len, unsigned char is_inverted, unsigned char is_measured)
{
    if (fft_len <= 0)
        return ERR_ARG_BOUNDS;

    fftwf_complex *fft_bfr;
    fftwf_plan plan;

    int i;

    fft_bfr = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fft_len);
    plan = fftwf_plan_dft_1d(fft_len, fft_bfr, fft_bfr,
                             FFTW_FORWARD, (is_measured ? FFTW_MEASURE : FFTW_ESTIMATE));

    while (fread(fft_bfr, sizeof(fftwf_complex), fft_len, instream) == fft_len)
    {
        if (is_inverted)
            for (i = 0; i < fft_len; i++)
                fft_bfr[i][1] = -fft_bfr[i][1];  // conjugate
        fftwf_execute(plan);
        //  write negative channels first ...
        fwrite(&fft_bfr[fft_len/2], sizeof(fftwf_complex), fft_len / 2, outstream);
        //  ... then positive channels
        fwrite(&fft_bfr[0], sizeof(fftwf_complex), fft_len / 2, outstream);
    }

    fftwf_destroy_plan(plan);
    fftwf_free(fft_bfr);

    return 0;
}

int sq_offset(FILE* instream, FILE* outstream, unsigned int nsamples, float real_delta, float imag_delta)
{
    if (nsamples <= 0)
        return ERR_ARG_BOUNDS;

    unsigned int smpli;
    float *smpls_bfr;

    smpls_bfr = malloc(nsamples * 4 * 2);
    if(smpls_bfr == NULL)
        return ERR_MALLOC;
    
    while (fread(smpls_bfr, 8, nsamples, instream) == nsamples)
    {
        for (smpli = 0; smpli < nsamples; smpli++)
        {
            smpls_bfr[(smpli<<1)+0] += real_delta;
            smpls_bfr[(smpli<<1)+1] += imag_delta;
        }

        fwrite(smpls_bfr, 8, nsamples, outstream);
    }

    free(smpls_bfr);

    return 0;
}

int sq_conjugate(FILE* instream, FILE* outstream, unsigned int nsamples)
{
    if (!((nsamples >= 2) && (nsamples <= MAX_SMPLS_LEN)))
        return ERR_ARG_BOUNDS;

    float *data_bfr;
    unsigned int datai;

    data_bfr = malloc(nsamples * sizeof(float) * 2);
    if(data_bfr == NULL)
        return ERR_MALLOC;

    while (fread(data_bfr, 8, nsamples, instream) == nsamples)
    {
        for (datai = 0; datai < nsamples; datai += 1)
            data_bfr[(datai<<1)+1] *= -1.0;

        fwrite(data_bfr, sizeof(float) * 2, nsamples, outstream);
    }

    free(data_bfr);

    return 0;
}

int sq_scaleandrotate(FILE* instream, FILE* outstream, unsigned int nsamples, float scale_factor, float radians)
{
    if (nsamples <= 0)
        return ERR_ARG_BOUNDS;

    unsigned int smpli;
    float *smpls_bfr;

    smpls_bfr = malloc(nsamples * 4 * 2);
    if(smpls_bfr == NULL)
        return ERR_MALLOC;

    float re, im;
    while (fread(smpls_bfr, 8, nsamples, instream) == nsamples)
    {
        for (smpli = 0; smpli < nsamples; smpli++)
        {
            re = smpls_bfr[(smpli<<1)+0];
            im = smpls_bfr[(smpli<<1)+1];
            smpls_bfr[(smpli<<1)+0] = scale_factor * (re * cos(radians) - im * sin(radians));
            smpls_bfr[(smpli<<1)+1] = scale_factor * (im * cos(radians) + re * sin(radians));
        }

        fwrite(smpls_bfr, 8, nsamples, outstream);
    }

    free(smpls_bfr);

    return 0;
}

int sq_scale(FILE* instream, FILE* outstream, unsigned int nsamples, float scale_factor)
{
    return sq_scaleandrotate(instream, outstream, nsamples, scale_factor, 0.0);
}

int sq_rotate(FILE* instream, FILE* outstream, unsigned int nsamples, float radians)
{
    return sq_scaleandrotate(instream, outstream, nsamples, 1.0, radians);
}

int sq_mix(FILE* instream, FILE* outstream, unsigned int nsamples, float radians)
{
    if (nsamples <= 0)
        return ERR_ARG_BOUNDS;

    unsigned int smpli;
    float *smpls_bfr;
    float angle = 0.0;

    const float TWO_PI = 2.0 * M_PI;

    radians = radians * -1.0;

    smpls_bfr = malloc(nsamples * 4 * 2);
    if(smpls_bfr == NULL)
        return ERR_MALLOC;

    float re, im;
    while (fread(smpls_bfr, 8, nsamples, instream) == nsamples)
    {
        for (smpli = 0; smpli < nsamples; smpli++)
        {
            angle += radians;
            angle = (angle > TWO_PI) ? (angle - TWO_PI) : angle;
            angle = (angle < -TWO_PI) ? (angle + TWO_PI) : angle;

            re = smpls_bfr[(smpli<<1)+0];
            im = smpls_bfr[(smpli<<1)+1];
            smpls_bfr[(smpli<<1)+0] = (re * (float)cos(angle) - im * (float)sin(angle));
            smpls_bfr[(smpli<<1)+1] = (im * (float)cos(angle) + re * (float)sin(angle));
        }

        fwrite(smpls_bfr, 8, nsamples, outstream);
    }

    free(smpls_bfr);

    return 0;
}

int sq_zoom(FILE* instream, FILE* outstream, unsigned int zoom_len)
{
    // TODO: Not properly tested yet
    if (!((zoom_len >= 2) && (zoom_len <= MAX_ZOOM_LEN)))
        return ERR_ARG_BOUNDS;

    float *input_bfr;
    float *output_bfr;

    int inbfri, outbfri;
    float sum_r, sum_i;

    outbfri = 0;

    input_bfr = malloc(zoom_len * sizeof(float) * 2);
    if(input_bfr == NULL)
        return ERR_MALLOC;
    output_bfr = malloc(ZOOM_OUTPUT_BFR_LEN * sizeof(float) * 2);
    if(output_bfr == NULL)
        return ERR_MALLOC;

    while (fread(input_bfr, sizeof(float) * 2, zoom_len, instream) == zoom_len)
    {
        sum_r = 0.0;
        sum_i = 0.0;
        for (inbfri = 0; inbfri < zoom_len; inbfri++)
        {
            sum_r += input_bfr[(inbfri<<1)+0];
            sum_i += input_bfr[(inbfri<<1)+1];
        }
        output_bfr[(outbfri<<1) + REAL] = sum_r;
        output_bfr[(outbfri<<1) + IMAG] = sum_i;
        outbfri++;

        if (!(outbfri < ZOOM_OUTPUT_BFR_LEN))
        {
            fwrite(output_bfr, sizeof(float) * 2, ZOOM_OUTPUT_BFR_LEN, outstream);
            outbfri = 0;
        }
    }

    free(input_bfr);
    free(output_bfr);

    return 0;
}

void init_window(float* wndwbfr, unsigned int wndwlen, unsigned int folds)
{
    unsigned int wndwi;
    unsigned int si;

    unsigned int sinusoids = (folds / 2) + 1;

    float h;

    for (wndwi = 0; wndwi < wndwlen; wndwi++)
    {
        h = 0.5 * (1.0 - cos((2.0 * M_PI * (float) wndwi) / (float)(wndwlen - 1)));
        wndwbfr[wndwi] = 0.0;
        for (si = 0; si < sinusoids; si++)
            wndwbfr[wndwi] += cos(((-2.0 * M_PI) / (float) wndwlen) *
                                  (si * (((float) wndwi) - (((float)(wndwlen - 1)) / 2.0))));
        wndwbfr[wndwi] *= h;
        wndwbfr[wndwi] /= sinusoids;
    }
}

int sq_wola(FILE* instream, FILE* outstream, unsigned int fftlen, unsigned int folds, unsigned int overlap, unsigned char is_window_dump)
{
    unsigned int wndwlen;

    float *wndwbfr;
    cmplx *readbfr;
    cmplx *smplbfr;
    cmplx *fftbfr;

    unsigned int wndwi, readi, smpli, ffti;
    unsigned int readlen;

    wndwlen = folds * fftlen;
    wndwbfr = malloc(wndwlen * sizeof(float));
    if(wndwbfr == NULL)
        return ERR_MALLOC;

    init_window(wndwbfr, wndwlen, folds);


    if (is_window_dump)
    {
        for (wndwi = 0; wndwi < wndwlen; wndwi++)
            fprintf(outstream, "%e\n", wndwbfr[wndwi]);
        free(wndwbfr);
        return 0;
    }

    readlen = fftlen;
    if (overlap == 25)
        readlen = (fftlen * 3) / 4;
    if (overlap == 50)
        readlen = (fftlen * 2) / 4;

    readbfr = malloc(readlen * sizeof(cmplx));
    if(readbfr == NULL)
        return ERR_MALLOC;
    smplbfr = malloc(wndwlen * sizeof(cmplx));
    if(smplbfr == NULL)
        return ERR_MALLOC;
    fftbfr = malloc(fftlen * sizeof(cmplx));
    if(fftbfr == NULL)
        return ERR_MALLOC;

    // initially fill the sample buffer to satisfy the first weight,
    // overlap, and add
    if (!(fread(smplbfr, sizeof(cmplx), wndwlen, instream) == wndwlen))
    {
        free(fftbfr);
        free(smplbfr);
        free(readbfr);
        free(wndwbfr);
        return ERR_STREAM_READ;
    }

    smpli = 0;

    for (;;)
    {
        for (ffti = 0; ffti < fftlen; ffti++)
            fftbfr[ffti][0] = fftbfr[ffti][1] = 0.0;
        ffti = 0;
        for (wndwi = 0; wndwi < wndwlen; wndwi++)
        {
            fftbfr[ffti][0] += wndwbfr[wndwi] * smplbfr[smpli][0];
            fftbfr[ffti][1] += wndwbfr[wndwi] * smplbfr[smpli][1];
            smpli++;
            if (!(smpli < wndwlen)) smpli = 0;
            ffti++;
            if (!(ffti < fftlen)) ffti = 0;
        }
        fwrite(fftbfr, sizeof(cmplx), fftlen, outstream);
        if (!(fread(readbfr, sizeof(cmplx), readlen, instream) == readlen))
            break;
        for (readi = 0; readi < readlen; readi++)
        {
            smplbfr[smpli][0] = readbfr[readi][0];
            smplbfr[smpli][1] = readbfr[readi][1];
            smpli++;
            if (!(smpli < wndwlen)) smpli = 0;
        }
    }

    free(fftbfr);
    free(smplbfr);
    free(readbfr);
    free(wndwbfr);

    return 0;
}
